/* Includes ------------------------------------------------------------------*/

#include "data_Processor.h"
#include "ai_platform.h"
#include "ai_utilities.h"
#include "network.h"
#include "network_data.h"
#include "app_x-cube-ai.h"

/* Imported Variable -------------------------------------------------------------*/

/* exported Variable -------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define AI_NETWORK_IN_1_HEIGHT  (24) 
#define AI_NETWORK_IN_1_WIDTH   ((ai_int)(AI_NETWORK_IN_1_SIZE/AI_NETWORK_IN_1_HEIGHT))
#define AI_NETWORK_IN_1_FORMAT   AI_BUFFER_FORMAT_FLOAT
#define AI_NETWORK_OUT_1_FORMAT  AI_BUFFER_FORMAT_FLOAT

#define N_OVERLAPPING_WIN     (1)
#define EXP_BETA_PARAM 0.2
/* Declaration of network objects from network.h
 -> ai_handle is a type void which points a memory space
 -> ai_u8 is an unsigned int on 1 byte
 */
/*Set the aligned attribute in GNU */
AI_ALIGNED(4)

static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
static ai_handle network = AI_HANDLE_NULL;
static ai_buffer ai_input[AI_NETWORK_IN_NUM] = { AI_NETWORK_IN_1 };
static ai_buffer ai_output[AI_NETWORK_OUT_NUM] = { AI_NETWORK_OUT_1 };
static uint8_t FinalResultCode = ID_NONE;
static ai_network_report report;

/* handling samples in data_sample_buffer */
static ai_size n_sample = 0;
static ai_float data_sample_buffer[N_OVERLAPPING_WIN * AI_NETWORK_IN_1_SIZE] = {
        0 };

/* Input : Array of float
 Return index of the most high value of the array
 */
static uint8_t MaxValue(const float * array, int size)
{
    float max = -1e9f;
    uint8_t max_idx = 0;
    for (int i = 0; i < size; ++i)
    {
        if (array[i] > max)
        {
            max = array[i];
            max_idx = i;
        }
    }
    return max_idx;
}

/*  Consider the previous result before updating the output.
 Use a parameter alpha which allows to manage the impact of the newest results
 by using the exponential average
 **/
static const float * exponential_average(float * scores, float alpha)
{
    static float last_scores[AI_NETWORK_OUT_1_SIZE] = { 0 };

    for (int i = 0; i < AI_NETWORK_OUT_1_SIZE; ++i)
    {
        last_scores[i] = (1.0f - alpha) * last_scores[i] + alpha * scores[i];
    }
    return last_scores;
}

/* Process the final Result*/
uint8_t Process_Result(float * scores)
{
    uint8_t result;
    result = MaxValue(exponential_average(scores, EXP_BETA_PARAM), AI_NETWORK_OUT_1_SIZE);
    return result;
}

/* Check the Network before using */
__STATIC_INLINE int aiCheckNetwork(const ai_network_report* report)
{
    if (!report)
        return -1;

    if (aiBufferSize(&report->activations) != AI_NETWORK_DATA_ACTIVATIONS_SIZE)
    {
        Printf(
                "defined activation buffer size is not coherent (expected=%d)\r\n",
                AI_NETWORK_DATA_ACTIVATIONS_SIZE);
        return -1;
    }

    if (aiBufferSize(&report->weights) != AI_NETWORK_DATA_WEIGHTS_SIZE)
    {
        Printf(
                "defined weights buffer size is not coherent (expected=%d)\r\n",
                AI_NETWORK_DATA_WEIGHTS_SIZE);
        return -1;
    }

    if ((AI_NETWORK_IN_NUM != report->n_inputs)
            || (AI_NETWORK_OUT_NUM != report->n_outputs)
            || (report->n_inputs != 1) || (report->n_outputs != 1))
    {
        Printf("only one input and one output is supported\r\n");
        return -1;
    }

    if ((ai_input[0].format != AI_NETWORK_IN_1_FORMAT)
            || (ai_output[0].format != AI_NETWORK_OUT_1_FORMAT))
    {
        Printf("input or output format unconsistancy\r\n");

        return -1;
    }

    if (AI_NETWORK_IN_1_WIDTH != ai_input[0].width)
    {
        Printf("input width unconsistancy\r\n");
        return -1;
    }

    if (AI_NETWORK_IN_1_HEIGHT != ai_input[0].height)
    {
        Printf(" input height unconsistancy\r\n");
        return -1;
    }

    return 0;
}

/* Exported Functions --------------------------------------------------------*/

/* Initialize the AI Network and enable the CRC clock for using AI library on stm32*/
int8_t DATA_InitProcesser(void)
{
    ai_error err;
    if (network != AI_HANDLE_NULL)
    {
        Printf("\r\nAI Network already initialized...\r\n");
        return -1;
    }

    FinalResultCode = ID_NONE;

    Printf("\r\nAI Network (AI platform API %d.%d.%d)...\r\n",
    AI_PLATFORM_API_MAJOR,
    AI_PLATFORM_API_MINOR,
    AI_PLATFORM_API_MICRO);

    /* enabling CRC clock for using AI libraries (for checking if STM32
     microprocessor is used)*/
    __HAL_RCC_CRC_CLK_ENABLE()
    ;

    /* create an instance of the network */
    Printf("Creating the network...\r\n");
    err = ai_network_create(&network, AI_NETWORK_DATA_CONFIG);
    if (err.type)
    {
        aiLogErr(err, "ai_network_create");
        return -3;
    }

    /* Query the created network to get relevant info from it */
    if (ai_network_get_info(network, &report))
    {
        aiPrintNetworkInfo(&report);

    } else
    {
        err = ai_network_get_error(network);
        aiLogErr(err, "ai_network_get_info");
        ai_network_destroy(&network);
        network = AI_HANDLE_NULL;
        return -4;
    }

    if (aiCheckNetwork(&report))
    {
        ai_network_destroy(&network);
        network = AI_HANDLE_NULL;
        return -5;
    }

    /* initialize the instance */
    Printf("Initializing the network...\r\n");

    /* build params structure to provide the references of the
     * activation and weight buffers */

    const ai_network_params params = {
    AI_NETWORK_DATA_WEIGHTS(ai_network_data_weights_get()),
    AI_NETWORK_DATA_ACTIVATIONS(activations) };

    if (!ai_network_init(network, &params))
    {
        err = ai_network_get_error(network);
        aiLogErr(err, "ai_network_init");
        ai_network_destroy(&network);
        network = AI_HANDLE_NULL;
        return -6;
    }

    FinalResultCode = ID_NONE;

    Printf("Initialized Neural Network\r\n");

    return 0;
}

/* ------------------------Run inference on the data collected -----------------------------------*/
uint8_t DATA_Infer(DATA_input_t ACC_Value)
{
    static ai_float out[AI_NETWORK_OUT_1_SIZE];
    ai_i32 batch;

    if (AI_HANDLE_NULL == network)
    {
        Printf("network handle is NULL\r\n");
        return ID_NONE ;
    }

    /* add samples of x, y and z in buffer */
    data_sample_buffer[n_sample++] = ACC_Value.AccX;
    data_sample_buffer[n_sample++] = ACC_Value.AccY;
    data_sample_buffer[n_sample++] = ACC_Value.AccZ;

    if (n_sample >= AI_NETWORK_IN_1_SIZE)
    {
        ai_input[0].data = AI_HANDLE_PTR(&data_sample_buffer);

        ai_output[0].data = AI_HANDLE_PTR(out);

        batch = ai_network_run(network, &ai_input[0], &ai_output[0]);

        if (batch != 1)
        {
            aiLogErr(ai_network_get_error(network), "ai_network_run");
        }

        FinalResultCode = Process_Result(out);
        n_sample = 0;
    }

    return FinalResultCode;

}

/* -------------------------------Return the final result -----------------------------------------------*/
uint8_t DATA_get_Result(void)
{
    return FinalResultCode;
}


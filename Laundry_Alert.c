/* TI RTOS and Simplelink includes */
#include <stdio.h>
#include <stdint.h>
#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC32XX.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/sysbios/BIOS.h>
#include "uart_term.h"
#include "unistd.h"

/* Temboo includes */
#include "temboo/Temboo.h"
#include "temboo/TembooUtil.h"
#include "temboo/http/TembooHttpSession.h"
#include "temboo/TembooTimer.h"
#include "device/DeviceGPIO.h"
#include "device/DeviceLog.h"
#include "device/DeviceGetMillis.h"

/* Local project includes */
#include "network_if.h"
#include "Board.h"
#include "TembooAccount.h"

#define MAX_WAIT 330000
#define LOOP_DELAY 100
#define BLINK_DELAY 1000

/* Temboo Session, holds static values for Temboo functions */
TembooHttpSession session;

/* Set timeout and interval values */
const int CHOREO_TIMEOUT = 60;

/* Array of Pin configurations for the TI driver */
GPIO_PinConfig gpioPinConfigs[] = {
    /* Push buttons for the launchpad */
    GPIOCC32XX_GPIO_22 | GPIO_CFG_INPUT | GPIO_CFG_IN_INT_RISING,
    GPIOCC32XX_GPIO_13 | GPIO_CFG_INPUT | GPIO_CFG_IN_INT_RISING,
    /* Pins used with Temboo program */
    GPIOCC32XX_GPIO_06 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,
    GPIOCC32XX_GPIO_14 | GPIO_CFG_INPUT,

};

/* Declaring constants to be used when using GPIO pins */
typedef enum CC3220_LAUNCHXL_GPIOName {
    CC3220_LAUNCHXL_GPIO_SW2 = 0,
    CC3220_LAUNCHXL_GPIO_SW3,
    CC3220_LAUNCHXL_GPIO_P61,
    CC3220_LAUNCHXL_GPIO_P62,
    CC3220_LAUNCHXL_GPIO_P05,

    CC3220_LAUNCHXL_GPIOCOUNT
} CC3220_LAUNCHXL_GPIOName;

GPIO_CallbackFxn gpioCallbackFunctions[] = {
    NULL, 
    NULL   
};

const GPIOCC32XX_Config GPIOCC32XX_config = {
    .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
    .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
    .intPriority = (~0)
};

/* Declare sensors variables and their configs */
TembooActuator tmb_red_led;
DeviceGPIOConfig tmb_red_ledConfig;
TembooActuator tmb_green_led;
DeviceGPIOConfig tmb_green_ledConfig;
TembooSensor tmb_vibration;
DeviceGPIOConfig tmb_vibrationConfig;

TembooError sessionSetup(TembooHttpSession* session) {

    initTemboo(1);

    /* Initialize Temboo session with account details */
    TembooError rc = initTembooHttpSession(session, 
            TEMBOO_ACCOUNT, 
            TEMBOO_APP_KEY_NAME, 
            TEMBOO_APP_KEY, 
            TEMBOO_DEVICE_TYPE);

    /* Initialize sensors and actuators */
    deviceDigitalGPIOInit(&tmb_red_ledConfig, &tmb_red_led, CC3220_LAUNCHXL_GPIO_P61, LOW, OUTPUT);
    deviceDigitalGPIOInit(&tmb_green_ledConfig, &tmb_green_led, CC3220_LAUNCHXL_GPIO_P62, LOW, OUTPUT);
    deviceDigitalGPIOInit(&tmb_vibrationConfig, &tmb_vibration, CC3220_LAUNCHXL_GPIO_P05, LOW, INPUT);

    return rc;
}

int sendChoreoRequest(TembooHttpSession* session, TembooChoreo* choreo) {
    UART_PRINT("Running Choreo\n");

    // Set Choreo Inputs
    ChoreoInput AuthTokenInput;
    AuthTokenInput.name = "AuthToken";
    AuthTokenInput.value = "**************";
    addChoreoInput(choreo, &AuthTokenInput);

    ChoreoInput FromInput;
    FromInput.name = "From";
    FromInput.value = "***********";
    addChoreoInput(choreo, &FromInput);

    ChoreoInput ToInput;
    ToInput.name = "To";
    ToInput.value = "************";
    addChoreoInput(choreo, &ToInput);

    ChoreoInput BodyInput;
    BodyInput.name = "Body";
    BodyInput.value = "Your laundry is finished! Go get it!";
    addChoreoInput(choreo, &BodyInput);

    ChoreoInput AccountSIDInput;
    AccountSIDInput.name = "AccountSID";
    AccountSIDInput.value = "*******************";
    addChoreoInput(choreo, &AccountSIDInput);

    /* runChoreoAsync will return a non-zero return code if it
    can not send the Choreo request */
    int rc = runChoreoAsync(choreo, session);

    if (rc != TEMBOO_SUCCESS) {
        UART_PRINT("Choreo error: %i\n\r", rc);
    }
    return rc;
}

void blink_green(void){
    tmb_green_led.write(&tmb_green_ledConfig, HIGH);
    usleep(BLINK_DELAY);
    tmb_green_led.write(&tmb_green_ledConfig, LOW);
    usleep(BLINK_DELAY);
}

int send_SMS(void){
    TembooChoreo choreo;
                
    /* Setting up send and receive buffers
     * Change size of buffers as needed */ 
    uint8_t rxBuff[1024] = {0};
    uint8_t txBuff[1024] = {0};

    /* Initialize the protocol */
    TembooHttpProtocol protocol;
    initHttpProtocol(&protocol, rxBuff, sizeof(rxBuff), txBuff, sizeof(txBuff));

    const char choreoName[] = "/Library/Twilio/SMSMessages/SendSMS";
    initHttpChoreo(&choreo, choreoName, &protocol);
    int rc = sendChoreoRequest(&session, &choreo);
    
    while (rc != TEMBOO_SUCCESS) {
        int rc = sendChoreoRequest(&session, &choreo);
        usleep(100);
    }
    
    /* Stop the Choreo and close the socket */
    stopTembooChoreo(&choreo);
    return 0; 
}

void* mainThread(void* args) {
    
    unsigned long now = 0;
    unsigned long time_since_vib = 0;
    int flag = 0;
    
    /* Connect to the WiFi router */
    while (0 != wifiConnect(WIFI_SSID, WPA_PASSWORD)) {
        UART_PRINT("Connection to router failed, trying again");
        sleep(2);
    }
    
    /* Write the Root CA needed to connect to Temboo */
    writeTembooRootCA();

    /* Update the device's time to the current time */
    while (updateCurrentTime("time-c.nist.gov") < 0) {
        // NIST requires a delay of at least 4 seconds between each query 
        sleep(4);
    }
    
    /* Initialize Temboo session */
    if (sessionSetup(&session) != TEMBOO_SUCCESS) {
        UART_PRINT("Temboo initialization failed");
        while (1);
    }
     
    while (1) {
        usleep(1000);
        
        // On initial iteration of the loop, set Millis to now.
        if (flag == 0){
            now = deviceGetMillis();
            flag = 1;
        }
        
        /* If vibration is detected, light red LED and reset value of now
         * to current millis.*/
        if (tmb_vibration.read(&tmb_vibrationConfig) ==  HIGH) {
            now = deviceGetMillis();
            tmb_red_led.write(&tmb_red_ledConfig, HIGH);
        } 
        
        /* Otherwise, keep track of time since last vibration. If it excceds
         * MAX_WAIT, send the SMS. */
        else{
            tmb_red_led.write(&tmb_red_ledConfig, LOW);
            time_since_vib = (deviceGetMillis() - now); 
            
            if (time_since_vib > MAX_WAIT){
                blink_green();
                send_SMS();
            }
        }
        usleep(LOOP_DELAY);
    }
}
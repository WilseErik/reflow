// Initializes all GPIO NC pins from pin nbr 'GPIO_NC_CURRENT' to 'GPIO_NC_END'
#include "gpio.h"

#if defined(GPIO_NC_CURRENT) && defined(GPIO_NC_END)
    #if NC_CURRENT < NC_END
        NC_DIR(NC_CURRENT) = DIR_OUT;
        NC_PIN(NC_CURRENT) = 0;
        
        #define GPIO_NC_TMP GPIO_NC_CURRENT
        #undef GPIO_NC_CURRENT
        #define GPIO_NC_CURRENT (GPIO_NC_TMP + 1)
        #undef GPIO_NC_TMP

        #include "gpio_set_nc_pins.h"
    #else
        #undef GPIO_NC_CURRENT
        #undef GPIO_NC_END
    #endif
#endif

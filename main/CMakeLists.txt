set(COMPONENT_SRCS "main.cpp")
set(COMPONENT_ADD_INCLUDEDIRS .) 
set(COMPONENT_REQUIRES  control
                        debug
                        espnow
                        led_strip
                        utils
                        task_now
                        task_bsp
                        task_tof) 

register_component()
set(COMPONENT_ADD_INCLUDEDIRS  . )
set(COMPONENT_SRCS  ./tsk_bsp.cpp )

list(APPEND EXTRA_COMPONENT_DIRS)

set(COMPONENT_PRIV_REQUIRES task_now)
set(COMPONENT_REQUIRES  led_strip vl53l0x color)
                        
register_component()

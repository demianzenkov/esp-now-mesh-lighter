set(COMPONENT_ADD_INCLUDEDIRS  . )
set(COMPONENT_SRCS  ./tsk_tof.cpp )

list(APPEND EXTRA_COMPONENT_DIRS)

set(COMPONENT_PRIV_REQUIRES task_now task_bsp)
set(COMPONENT_REQUIRES  vl53l0x)
                        
register_component()

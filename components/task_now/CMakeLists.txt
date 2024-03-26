set(COMPONENT_ADD_INCLUDEDIRS  . )
set(COMPONENT_SRCS  ./tsk_now.cpp )

list(APPEND EXTRA_COMPONENT_DIRS)

set(COMPONENT_PRIV_REQUIRES task_bsp)
set(COMPONENT_REQUIRES control espnow)
                        
register_component()

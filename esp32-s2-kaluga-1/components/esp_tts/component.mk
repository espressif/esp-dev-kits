COMPONENT_ADD_INCLUDEDIRS := include 

COMPONENT_SRCDIRS := ./ 

LIB_FILES := $(shell ls $(COMPONENT_PATH)/lib*.a) \

LIBS := $(patsubst lib%.a,-l%,$(LIB_FILES))

COMPONENT_ADD_LDFLAGS +=  -L$(COMPONENT_PATH) \
						   $(LIBS)


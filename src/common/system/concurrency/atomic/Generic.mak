# Generic Makefile framework
# Author: Chen Feng <chen3feng@hotmail.com>
# Last Update: Jan 09 2009
# Version 1.00

ifeq ($(DEBUG),)
CPPFLAGS += -O3
endif

ifneq ($(PROF),)
CPPFLAGS += -pg
LDFLAGS += -pg
endif

CPPFLAGS := -g -pipe -Wall $(CPPFLAGS)

CFLAGS := $(CPPFLAGS) $(CFLAGS)
CXXFLAGS := $(CPPFLAGS) $(CXXFLAGS)

LDFLAGS += -g -pipe
ARFLAGS = crs

sources ?= $(foreach target,$(targets),$($(target).sources))

objdir ?= .objs
depdir ?= $(objdir)

VPATH = $(dir $(sources))

objects	= $(addprefix $(objdir)/,$(addsuffix .o,$(notdir $(sources))))
depends = $(addprefix $(depdir)/,$(addsuffix .d,$(notdir $(sources))))

###############################################################################
# Functions
###############################################################################

get_cxx_flags = $(if $($(notdir $(1)).CXXFLAGS),$($(notdir $(1)).CXXFLAGS),$(CXXFLAGS)) $($(notdir $(1)).EXTRA_CXXFLAGS)
get_c_flags = $(if $($(notdir $(1)).CFLAGS),$($(notdir $(1)).CFLAGS),$(CFLAGS)) $($(notdir $(1)).EXTRA_CFLAGS)
get_ld_flags = $(if $($(notdir $(1)).LDFLAGS),$($(notdir $(1)).LDFLAGS),$(LDFLAGS)) $($(notdir $(1)).EXTRA_LDFLAGS)
get_ar_flags = $(if $($(notdir $(1)).ARFLAGS),$($(notdir $(1)).ARFLAGS),$(ARFLAGS)) $($(notdir $(1)).EXTRA_ARFLAGS)
get_objects = $(addprefix $(objdir)/,$(addsuffix .o,$(notdir $(1))))
get_target_name = $(1)

# echo text in color mode
# params:
# $(1) fd
# $(2) color
# $(3) text
define color_echo
	if [ -t $(1) ]; then\
		echo -e "\\033[$(2)m$(3)\\033[m" >>/dev/fd/$(1);\
	else\
		echo -e "$(3)" >>/dev/fd/$(1);\
	fi;
endef

# run command and echo output in color according result
define color_run
	o=`$(1) 2>&1`;\
	ret=$$?;\
	if [ $$ret -eq 0 ]; then\
		if [ -z "$$o" ]; then\
			$(call color_echo,1,1;32,$(1))\
		else\
			$(call color_echo,2,1;33,$(1))\
			$(call color_echo,2,1;33,$$o)\
		fi\
	else\
		$(call color_echo,2,1;31,$(1))\
		$(call color_echo,2,1;31,$$o)\
	fi;\
	exit $$ret;
endef

define build_target
ifeq ($$($(1).type),)
$(1): $$(call get_objects,$$($(1).sources))
	@$$(call color_echo,1,1,"Linking program $(1) ...")
	@$$(call color_run,$$(CXX) -o $$@ $$^ $$(call get_ld_flags,$(1)))
else
ifeq ($$($(1).type),shared)
$(1): $$(call get_objects,$$($(1).sources))
	@$$(call color_echo,1,1,"Linking shared library $(1) ...")
	@$$(call color_run,$$(CXX) -shared -o $$@ $$^ $$(call get_ld_flags,$(1)))
else
ifeq ($$($(1).type),library)
$(1): $$(call get_objects,$$($(1).sources))
	@$$(call color_echo,1,1,"Linking static library $(1) ...")
	@$$(call color_run,$$(AR) $$(call get_ar_flags,$(1)) $$@ $$^ )
endif
endif
endif
endef

#else
#ifeq ($$($(1).type),dir)
#force:;
#$(1): force
#	$(MAKE) -C $(1)
#endif

###############################################################################
# Rules
###############################################################################

.PHONY: all clean distclean

all: $(targets)

$(objdir):
	@mkdir -p $@

$(depends) : $(depdir)


$(depdir)/%.cpp.d : %.cpp
	@mkdir -p $(depdir)
	@$(CC) -c -MM -MT $(depdir)/$(notdir $<).o $(call get_cxx_flags,$<) $< | sed 's|^\($(depdir)/$(notdir $<).o\)|\1 $@|' > $@

$(depdir)/%.cxx.d : %.cxx
	@mkdir -p $(depdir)
	@$(CC) -c -MM -MT $(depdir)/$(notdir $<).o $(call get_cxx_flags,$<) $< | sed 's|^\($(depdir)/$(notdir $<).o\)|\1 $@|' > $@

$(depdir)/%.cc.d : %.cc
	@mkdir -p $(depdir)
	@$(CC) -c -MM -MT $(depdir)/$(notdir $<).o $(call get_cxx_flags,$<) $< | sed 's|^\($(depdir)/$(notdir $<).o\)|\1 $@|' > $@

$(depdir)/%.c.d : %.c
	@mkdir -p $(depdir)
	@$(CC) -c -MM -MT $(depdir)/$(notdir $<).o $(call get_c_flags,$<) $< | sed 's|^\($(depdir)/$(notdir $<).o\)|\1 $@|' > $@

$(objdir)/%.cpp.o : %.cpp
	@mkdir -p $(objdir)
	@$(call color_run,$(CC) -c -o $@ $(call get_cxx_flags,$<) $<)

$(objdir)/%.cxx.o : %.cxx
	@mkdir -p $(objdir)
	@$(call color_run,$(CC) -c -o $@ $(call get_cxx_flags,$<) $<)

$(objdir)/%.cc.o : %.cc
	@mkdir -p $(objdir)
	@$(call color_run,$(CC) -c -o $@ $(call get_cxx_flags,$<) $<)

$(objdir)/%.c.o : %.c
	@mkdir -p $(objdir)
	@$(call color_run,$(CC) -c -o $@ $(call get_c_flags,$<) $<)

depends: $(depends)

$(foreach target,$(targets),$(eval $(call build_target,$(target))))

clean:
	@$(RM) $(objects) $(targets)

distclean: clean
	@$(RM) -r $(objdir) $(scriptdir)

realclean: distclean

ifneq ($(depends),)
-include $(depends)
endif

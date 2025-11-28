CXX			:= g++

RM			:= rm -f
MKDIR		:= mkdir -p
RMDIR		:= rmdir

CXXFLAGS	:= -std=c++11 -O3 -I./third_party/cpp-httplib -I./third_party/json/include
CXXDFLAGS	:= -std=c++11 -g -Wall -I./third_party/cpp-httplib -I./third_party/json/include
LDFLAGS		:=

BUILDDIR	:= ./build
OBJDIR		:= $(BUILDDIR)/obj
BINDIR		:= $(BUILDDIR)/bin

VPATH		:= console:mahjong:utils:server

CXXINCLUDE	:= $(patsubst %,-I%,$(subst :, ,$(VPATH)))

Dirs		:= $(OBJDIR) $(BINDIR)

Example		:= $(BINDIR)/example
Example		:= ./calc_server
Objects		:= $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(wildcard *.cpp */*.cpp)))


all: $(Example)

$(OBJDIR)/%.o: %.cpp | $(Dirs)
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -c $< -o $@

$(OBJDIR)/%.d: %.cpp | $(Dirs)
	@set -e; $(RM) $@; \
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -MM $< > $@.$$$$; \
	$(RM) $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
endif

$(Example): $(Objects)
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -o $@ $^ $(LDFLAGS)

$(Check):	$(filter-out $(OBJDIR)/$(subst $(BINDIR)/,,$(Example).o), $(Objects))
	$(CXX) $(CXXFLAGS) $(CXXINCLUDE) -o $@ $^ $(LDFLAGS)

debug: CXXFLAGS := $(CXXDFLAGS)
debug: $(Example)

run: $(Example)
	$(Example)

check:
	@echo "unit_test已取消编译，无可执行文件。"

check: $(Check)
	$(Check)

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) $(Example)
	-$(RMDIR) $(OBJDIR)
	-$(RMDIR) $(BINDIR)
	-$(RMDIR) $(BUILDDIR)
	-$(RMDIR) $(OBJDIR)
	-$(RMDIR) $(BINDIR)
	-$(RMDIR) $(BUILDDIR)

.PHONY: all debug run check clean

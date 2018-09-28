#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/core/c4.o \
	${OBJECTDIR}/core/extras.o \
	${OBJECTDIR}/core/main.o \
	${OBJECTDIR}/core/syscalls.o \
	${OBJECTDIR}/platform/scheme/scheme.o \
	${OBJECTDIR}/platform/scheme/scheme_internal.o


# C Compiler Flags
CFLAGS=-Wno-format -rdynamic

# CC Compiler Flags
CCFLAGS=-Wno-format -rdynamic -Wall -Werror
CXXFLAGS=-Wno-format -rdynamic -Wall -Werror

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/core/c4.o: core/c4.c 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/c4.o core/c4.c

${OBJECTDIR}/core/extras.o: core/extras.c 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/extras.o core/extras.c

${OBJECTDIR}/core/main.o: core/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCPP14 -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/main.o core/main.cpp

${OBJECTDIR}/core/syscalls.o: core/syscalls.c 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/syscalls.o core/syscalls.c

${OBJECTDIR}/platform/scheme/scheme.o: platform/scheme/scheme.c 
	${MKDIR} -p ${OBJECTDIR}/platform/scheme
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/platform/scheme/scheme.o platform/scheme/scheme.c

${OBJECTDIR}/platform/scheme/scheme_internal.o: platform/scheme/scheme_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}/platform/scheme
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCPP14 -DDEBUG -DGCC_RDYNAMIC -Iinclude -Inon-contrib/string_view-standalone/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/platform/scheme/scheme_internal.o platform/scheme/scheme_internal.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

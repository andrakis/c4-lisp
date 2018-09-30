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
	${OBJECTDIR}/scheme.o \
	${OBJECTDIR}/scheme_internal.o \
	${OBJECTDIR}/syscalls.o


# C Compiler Flags
CFLAGS=-Wno-format -rdynamic -Wall -Werror -Wno-error=unused-function

# CC Compiler Flags
CCFLAGS=-Wno-format -rdynamic -Wall -Werror -Wno-error=unused-function
CXXFLAGS=-Wno-format -rdynamic -Wall -Werror -Wno-error=unused-function

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../rtl/${CND_PLATFORM}/libscheme.${CND_CONF}.${CND_DLIB_EXT}

../../rtl/${CND_PLATFORM}/libscheme.${CND_CONF}.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../../rtl/${CND_PLATFORM}
	${LINK.cc} -o ../../rtl/${CND_PLATFORM}/libscheme.${CND_CONF}.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/scheme.o: scheme.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -I../../include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/scheme.o scheme.c

${OBJECTDIR}/scheme_internal.o: scheme_internal.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -s -DCPP14 -DDEBUG -DRDYNAMIC -I../../include -std=c++14 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/scheme_internal.o scheme_internal.cpp

${OBJECTDIR}/syscalls.o: syscalls.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -DGCC_RDYNAMIC -I../../include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syscalls.o syscalls.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../rtl/${CND_PLATFORM}/libscheme.${CND_CONF}.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

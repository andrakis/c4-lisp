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
	${OBJECTDIR}/core/syscalls.o


# C Compiler Flags
CFLAGS=-rdynamic

# CC Compiler Flags
CCFLAGS=-rdynamic -Wall
CXXFLAGS=-rdynamic -Wall

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/core/c4.o: core/c4.c 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.c) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DDEBUG -DGCC_RDYNAMIC -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/c4.o core/c4.c

${OBJECTDIR}/core/extras.o: core/extras.c 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.c) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DDEBUG -DGCC_RDYNAMIC -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/extras.o core/extras.c

${OBJECTDIR}/core/main.o: core/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DCPP14 -DDEBUG -DGCC_RDYNAMIC -Iinclude -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/main.o core/main.cpp

${OBJECTDIR}/core/syscalls.o: core/syscalls.cpp 
	${MKDIR} -p ${OBJECTDIR}/core
	${RM} "$@.d"
	$(COMPILE.cc) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DCPP14 -DDEBUG -DGCC_RDYNAMIC -Iinclude -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/core/syscalls.o core/syscalls.cpp

# Subprojects
.build-subprojects:
	cd platform/scheme && ${MAKE}  -f Makefile CONF=Debug
	cd c4_modules/c4-assembler && ${MAKE}  -f Makefile CONF=Debug
	cd c4_modules/c4-internal && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lisp4

# Subprojects
.clean-subprojects:
	cd platform/scheme && ${MAKE}  -f Makefile CONF=Debug clean
	cd c4_modules/c4-assembler && ${MAKE}  -f Makefile CONF=Debug clean
	cd c4_modules/c4-internal && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

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
	${OBJECTDIR}/lib_src/libinternal.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../lib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../lib/lib${PROJECTNAME}.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

../../lib/lib${PROJECTNAME}.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../../lib
	${LINK.c} -o ../../lib/lib${PROJECTNAME}.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/lib_src/libinternal.o: lib_src/libinternal.c 
	${MKDIR} -p ${OBJECTDIR}/lib_src
	${RM} "$@.d"
	$(COMPILE.c) -g -DCND_PLATFORM=\"${CND_PLATFORM}\" -DCONF_${CND_CONF}=1 -DPROJECTNAME=\"${PROJECTNAME}\" -I../../include -I./include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lib_src/libinternal.o lib_src/libinternal.c

# Subprojects
.build-subprojects:
	cd ../../c4-internal && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../lib/lib${PROJECTNAME}.${CND_CONF}-${CND_PLATFORM}.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:
	cd ../../c4-internal && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
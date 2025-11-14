#!/bin/sh
# Minimal automated build script for the provided FreeBSD sh subtree.
# Assumes source tree is at ~/sh-src (adjust SRCDIR), POSIX toolchain installed.
# Produces a standalone `sh` in OUTDIR. Does not use FreeBSD build system.

set -e

# === Config adjust as needed ===
SRCDIR="${PWD}/ash"          # location of the provided Makefile and .c/.h files
OUTDIR="${PWD}/obj/ash"      # where .o and final binary go
CHMOD="${CHMOD:-chmod}"
BIN_MODE="${BIN_MODE:-751}"
TOUCH="${TOUCH:-touch}"      # needs -r and -h options
AR="${AR:-ar}"
CC="${CC:-cc}"
CFLAGS="-O2 -DSHELL -I${SRCDIR} -I. -ffunction-sections -fdata-sections -fPIC"
LDFLAGS="-fuse-ld=lld -fPIE"
ASH_LINE_LIB="${ASH_LINE_LIB:-readline}"
LIBS="-ledit"     # set to "" if libedit not available
if [ -n $ASH_LINE_LIB ]; then
	# Function to check for weak linking support
	check_weak_link() {
		local TEMP_SRC=".linker_dummy.c"
		echo "int main() { return 0; }" > ${TEMP_SRC} ;
		if ${CC} -Wl,-weak-l${ASH_LINE_LIB} ${TEMP_SRC} -o /dev/null 2>/dev/null; then
			echo "-Wl,-weak-l"
		else
			echo "-l"
		fi
		rm -f ${TEMP_SRC} 2>/dev/null ;
		unset TEMP_SRC ;
	}
	# Get the appropriate linker flag
	LINKER_FLAG=$(check_weak_link)
	LIBS="${LIBS} ${LINKER_FLAG}${ASH_LINE_LIB}"
fi
if [ -x $(which "${YACC:-yacc}") ]; then
	YACC="${YACC:-yacc}"     # or bison -y
else
  if [ -x $(which "bison") ]; then
    YACC="bison -y"          # or bison -y
  fi
fi
LEX="${LEX:-flex}"           # or lex
# ================================

mkdir -p "${OUTDIR}"
mkdir -p "${OUTDIR}/bltin"
cd "${SRCDIR}"

# 1) Build generator tools from their .c if present
for tool in mknodes mksyntax mktokens mkbuiltins; do
  src="${tool}.c"
  if [ -f "${src}" ]; then
    echo "building generator: ${src}"
    ${CC} ${CFLAGS} -o "${OUTDIR}/${tool}" "${SRCDIR}/${src}"
  else
    if [ -f "${SRCDIR}/${tool}" ] ; then
      cp -vf "${SRCDIR}/${tool}" "${OUTDIR}/${tool}"
    fi
  fi
  # Optional: reproducible improvements
  ${CHMOD} ${BIN_MODE} "${OUTDIR}/${tool}" || true
  ${TOUCH} -r "${OUTDIR}" -h "${OUTDIR}/${tool}" || true
done

# PATCHED Build shims from their .c if present
SHIM_LIBS="${SHIM_LIBS} -L${OUTDIR}"
for tool in eaccess setmode; do
  src="${tool}_shim.c"
  hdr="${tool}_shim.h"
  lib="${tool}.a"
  if [ -f "${src}" ]; then
    echo "building shim: ${src}"
    if [ -f "${hdr}" ] ; then
      ${CC} --std=c11 -ffunction-sections -fdata-sections -fPIC -fcommon -I${SRCDIR} -I. -fkeep-static-consts -c "${SRCDIR}/${src}" -o "${OUTDIR}/${tool}"
    else
      ${CC} --std=c11 -ffunction-sections -fdata-sections -fPIC -fcommon -I${SRCDIR} -c "${SRCDIR}/${src}" -o "${OUTDIR}/${tool}"
    fi
    ${AR} rcs "${OUTDIR}/${lib}" "${OUTDIR}/${tool}"
    SHIM_LIBS="${lib} ${SHIM_LIBS}"
    ${CHMOD} ${BIN_MODE} "${OUTDIR}/${lib}" || true
    ${TOUCH} -r "${OUTDIR}" -h "${OUTDIR}/${lib}" || true
  fi
done

# set LIBS with shims

LIBS="${SHIM_LIBS} ${LIBS}"

# Use local tools from OUTDIR when invoking
PATH="${OUTDIR}:$PATH"
export PATH

# 2) Run generator tools (if present) to create generated sources/headers
if [ -x "${OUTDIR}/mknodes" ]; then
  echo "running mknodes..."
  "${OUTDIR}/mknodes" "${SRCDIR}/nodetypes" "${SRCDIR}/nodes.c.pat"
fi
if [ -x "${OUTDIR}/mksyntax" ]; then
  echo "running mksyntax..."
  "${OUTDIR}/mksyntax"
fi
if [ -x "${OUTDIR}/mktokens" ]; then
  echo "running mktokens..."
  "${OUTDIR}/mktokens"
fi
if [ -x "${OUTDIR}/mkbuiltins" ]; then
  echo "running mkbuiltins..."
  "${OUTDIR}/mkbuiltins" "${SRCDIR}"
fi

# 3) Generate yacc/lex outputs if needed
if [ -f arith_yacc.y ] && [ ! -f arith_yacc.c ]; then
  ${YACC:-yacc} -d -o arith_yacc.c arith_yacc.y
fi
if [ -f arith_yylex.l ] && [ ! -f arith_yylex.c ]; then
  ${LEX:-flex} -o arith_yylex.c arith_yylex.l
fi

# 4) Compile sources
SRCS="
alias.c arith_yacc.c arith_yylex.c cd.c bltin/echo.c error.c eval.c \
exec.c expand.c histedit.c input.c jobs.c kill.c mail.c main.c memalloc.c \
miscbltin.c mystring.c options.c output.c parser.c printf.c redir.c show.c \
test.c trap.c var.c builtins.c nodes.c syntax.c
"
OBJLIST=""
for s in $SRCS; do
  [ -f "${SRCDIR}/${s}" ] || { echo "skipping missing ${s}"; continue; }
  obj="${OUTDIR}/${s%.c}.o"
  EXTRA_CFLAGS="-Wall"
  if [ ${s} == *echo.c* ] ; then
    EXTRA_CFLAGS="-I${SRCDIR}/bltin"
  fi
  if [ ${s} == *kill.c* ] || [ ${s} == *trap.c* ]; then
    if [ -d "${SRCDIR}/../musl/SignalWright/SignalWright/include" ]; then
      EXTRA_CFLAGS="-I${SRCDIR}/../musl/SignalWright/SignalWright/include"
    fi
  fi
  if [ ${s} == *eval.c* ] || [ ${s} == *input.c* ] || [ ${s} == *output.c* ] || [ ${s} == *mail.c* ] ; then
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -Wno-implicit-function-declaration"
  fi
  if [ ${s} == *input.c* ] || [ ${s} == *output.c* ] || [ ${s} == *mail.c* ] ; then
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -Wno-int-conversion"
  fi
  echo "compiling ${s}"
  ${CC} ${CFLAGS} ${EXTRA_CFLAGS} -c "${SRCDIR}/${s}" -o "${obj}"
  OBJLIST="${OBJLIST} ${obj}"
  ${CHMOD} ${BIN_MODE} "${obj}" || true
  ${TOUCH} -r "${OUTDIR}" -h "${obj}" || true
done

# 5) Link
echo "linking sh..."
cd "${OUTDIR}"
#-weak-leditline
if ${CC} -o sh -fPIE ${OBJLIST} ${LDFLAGS} ${LIBS} 2>/dev/null; then
  echo "linked successfully with ${LIBS}"
else
  echo "linking failed, showing verbose output:"
  ${CC} -o sh -fPIE ${OBJLIST} ${LDFLAGS} ${LIBS} || true
  exit 2;
fi

# 6) Test binary quickly
if [ -x ./sh ]; then
  echo "build succeeded: ${OUTDIR}/sh"
  ./sh -c 'echo sh_ok' 2>/dev/null || echo "built sh did not run 'echo sh_ok' successfully" >&2
else
  echo "sh binary not found after link"
  exit 1
fi

# 7) Optional: stage-install to DESTDIR if requested via env var DESTDIR
if [ -n "${DESTDIR}" ]; then
  dst="${DESTDIR}"
  mkdir -p "${dst}/bin"
  echo "installing to ${dst}/bin/sh"
  install -m 755 ./sh "${dst}/bin/sh"
fi

echo "done."

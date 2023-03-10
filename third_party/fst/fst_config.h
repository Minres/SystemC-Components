/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix). */
#if !defined(__MINGW32__) && !defined(__FreeBSD__)
# define HAVE_ALLOCA_H 1
#endif

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/*
 * possible disables:
 *
 * FST_DYNAMIC_ALIAS_DISABLE : dynamic aliases are not processed
 * FST_DYNAMIC_ALIAS2_DISABLE : new encoding for dynamic aliases is not generated
 * FST_WRITEX_DISABLE : fast write I/O routines are disabled
 *
 * possible enables:
 *
 * FST_DEBUG : not for production use, only enable for development
 * FST_REMOVE_DUPLICATE_VC : glitch removal (has writer performance impact)
 * HAVE_LIBPTHREAD -> FST_WRITER_PARALLEL : enables inclusion of parallel writer code
 * FST_DO_MISALIGNED_OPS (defined automatically for x86 and some others) : CPU architecture can handle misaligned loads/stores
 * _WAVE_HAVE_JUDY : use Judy arrays instead of Jenkins (undefine if LGPL is not acceptable)
 *
 */

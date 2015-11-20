
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <string.h>

#ifdef USE_TBM_BUFFER
#include <mmf/mm_types.h>
#include <tbm_type.h>
#include <tbm_surface.h>
#include <tbm_bufmgr.h>
#endif

typedef struct _GstMMVideoMemory                GstMMVideoMemory;
typedef struct _GstMMVideoMemoryAllocator       GstMMVideoMemoryAllocator;
typedef struct _GstMMVideoMemoryAllocatorClass  GstMMVideoMemoryAllocatorClass;
typedef struct _GstMMBuffer                     GstMMBuffer;


struct _GstMMVideoMemory
{
    GstMemory mem;
    GstMMBuffer *mm_video_buffer;
};

struct _GstMMVideoMemoryAllocator
{
  GstAllocator parent;
};

struct _GstMMVideoMemoryAllocatorClass
{
  GstAllocatorClass parent_class;
};

struct _GstMMBuffer
{
    MMVideoBuffer mm_buffer;
    gint video_width;
    gint video_height;
    gint video_alignment;
};

#define GST_MM_VIDEO_MEMORY_TYPE "mmvideobuffer"


static GQuark gst_mm_buffer_data_quark = 0;

#define GST_MM_BUFFER_POOL(pool) ((GstMMBufferPool *) pool)
typedef struct _GstMMBufferPool GstMMBufferPool;
typedef struct _GstMMBufferPoolClass GstMMBufferPoolClass;

struct _GstMMBufferPool
{
  GstVideoBufferPool parent;

  GstElement *element;

  GstCaps *caps;
  gboolean add_videometa;
  GstVideoInfo video_info;

  GstAllocator *allocator;

  /* Set from outside this pool */
  /* TRUE if we're currently allocating all our buffers */
  gboolean allocating;

  /* TRUE if the pool is not used anymore */
  gboolean deactivated;

  /* For populating the pool from another one */
  GstBufferPool *other_pool;
  GPtrArray *buffers;
  GQueue *mm_buffers;

  /* Used during acquire for output ports to
   * specify which buffer has to be retrieved
   * and during alloc, which buffer has to be
   * wrapped
   */
  gint current_buffer_index;

#ifdef USE_TBM_BUFFER
  tbm_bufmgr hTBMBufMgr;
#endif

};

struct _GstMMBufferPoolClass
{
  GstVideoBufferPoolClass parent_class;
};

GstBufferPool *
gst_mm_buffer_pool_new (GstElement * element );

#ifdef USE_TBM_BUFFER

/*MFC Buffer alignment macros*/
#define S5P_FIMV_DEC_BUF_ALIGN                  (8 * 1024)
#define S5P_FIMV_ENC_BUF_ALIGN                  (8 * 1024)
#define S5P_FIMV_NV12M_HALIGN                   16
#define S5P_FIMV_NV12M_LVALIGN                  16
#define S5P_FIMV_NV12M_CVALIGN                  8
#define S5P_FIMV_NV12MT_HALIGN                  128
#define S5P_FIMV_NV12MT_VALIGN                  64
#define S5P_FIMV_NV12M_SALIGN                   2048
#define S5P_FIMV_NV12MT_SALIGN                  8192

#define ALIGN(x, a)       (((x) + (a) - 1) & ~((a) - 1))

/* Buffer alignment defines */
#define SZ_1M                                   0x00100000
#define S5P_FIMV_D_ALIGN_PLANE_SIZE             64

#define S5P_FIMV_MAX_FRAME_SIZE                 (2 * SZ_1M)
#define S5P_FIMV_NUM_PIXELS_IN_MB_ROW           16
#define S5P_FIMV_NUM_PIXELS_IN_MB_COL           16

/* Macro */
#define ALIGN_TO_4KB(x)   ((((x) + (1 << 12) - 1) >> 12) << 12)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define CHOOSE_MAX_SIZE(a,b) ((a) > (b) ? (a) : (b))

#endif


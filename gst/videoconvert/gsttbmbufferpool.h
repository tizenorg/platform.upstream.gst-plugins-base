
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include <string.h>

#ifdef USE_TBM_BUFFER
#include <mm_types.h>
#include <tbm_type.h>
#include <tbm_surface.h>
#include <tbm_bufmgr.h>
#endif

#define ALIGN(x, a)       (((x) + (a) - 1) & ~((a) - 1))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

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


#if 0

static GQuark gst_tbm_buffer_data_quark = 0;


typedef struct _GstTBMBuffer GstTBMBuffer;
struct _GstTBMBuffer
{
    GstBuffer *buffer;
    void *mmBuffer;
};

#define GST_TBM_BUFFER_POOL(pool) ((GstTBMBufferPool *) pool)
typedef struct _GstTBMBufferPool GstTBMBufferPool;
typedef struct _GstTBMBufferPoolClass GstTBMBufferPoolClass;

struct _GstTBMBufferPool
{
  GstVideoBufferPool parent;

  GstElement *element;

  GstCaps *caps;
  gboolean add_videometa;
  GstVideoInfo video_info;

  /* Owned by element, element has to stop this pool before
   * it destroys component or port */
  GstElement *component;

  GstAllocator *allocator;

  /* Set from outside this pool */
  /* TRUE if we're currently allocating all our buffers */
  gboolean allocating;

  /* TRUE if the pool is not used anymore */
  gboolean deactivated;

  /* For populating the pool from another one */
  GstBufferPool *other_pool;
  GPtrArray *buffers;

  /* Used during acquire for output buffer to
   * specify which buffer has to be retrieved
   * and during alloc, which buffer has to be
   * wrapped
   */
  gint current_buffer_index;
};

struct _GstTBMBufferPoolClass
{
  GstVideoBufferPoolClass parent_class;
};

GType gst_tbm_buffer_pool_get_type (void);

G_DEFINE_TYPE (GstTBMBufferPool, gst_tbm_buffer_pool, GST_TYPE_BUFFER_POOL);

static gboolean
gst_tbm_buffer_pool_start (GstBufferPool * bpool)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);

  /* Only allow to start the pool if we still are attached
   * to a component and port */
  GST_OBJECT_LOCK (pool);
  if (!pool->component ) {
    GST_OBJECT_UNLOCK (pool);
    return FALSE;
  }
  GST_OBJECT_UNLOCK (pool);

  return
      GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->start (bpool);
}

static gboolean
gst_tbm_buffer_pool_stop (GstBufferPool * bpool)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);

  /* Remove any buffers that are there */
  g_ptr_array_set_size (pool->buffers, 0);

  if (pool->caps)
    gst_caps_unref (pool->caps);
  pool->caps = NULL;

  pool->add_videometa = FALSE;

  return GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->stop (bpool);
}

static const gchar **
gst_tbm_buffer_pool_get_options (GstBufferPool * bpool)
{
  static const gchar *raw_video_options[] =
      { GST_BUFFER_POOL_OPTION_VIDEO_META, NULL };
  static const gchar *options[] = { NULL };
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);

  return options;
}

static gboolean
gst_tbm_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);
  GstCaps *caps;

  GST_OBJECT_LOCK (pool);

  if (!gst_buffer_pool_config_get_params (config, &caps, NULL, NULL, NULL))
    goto wrong_config;

  if (caps == NULL)
    goto no_caps;

  if (pool->caps)
    gst_caps_unref (pool->caps);
  pool->caps = gst_caps_ref (caps);

  GST_OBJECT_UNLOCK (pool);

  return GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->set_config
      (bpool, config);

  /* ERRORS */
wrong_config:
  {
    GST_OBJECT_UNLOCK (pool);
    GST_WARNING_OBJECT (pool, "invalid config");
    return FALSE;
  }
no_caps:
  {
    GST_OBJECT_UNLOCK (pool);
    GST_WARNING_OBJECT (pool, "no caps in config");
    return FALSE;
  }
wrong_video_caps:
  {
    GST_OBJECT_UNLOCK (pool);
    GST_WARNING_OBJECT (pool,
        "failed getting geometry from caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }
}

static GstFlowReturn
gst_tbm_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);
  GstBuffer *buf;
  GstTBMBuffer *tbm_buf;
  GstMemory *mem;

  g_return_val_if_fail (pool->allocating, GST_FLOW_ERROR);

  tbm_buf = g_ptr_array_index (pool->buffers, pool->current_buffer_index);
  g_return_val_if_fail (tbm_buf != NULL, GST_FLOW_ERROR);

    mem = gst_tbm_memory_allocator_alloc (pool->allocator, 0, tbm_buf);
    buf = gst_buffer_new ();
    gst_buffer_append_memory (buf, mem);
    g_ptr_array_add (pool->buffers, buf);

  gst_mini_object_set_qdata (GST_MINI_OBJECT_CAST (buf),
      gst_tbm_buffer_data_quark, tbm_buf, NULL);

  *buffer = buf;

  pool->current_buffer_index++;

  return GST_FLOW_OK;
}

static void
gst_tbm_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);

  /* If the buffers belong to another pool, restore them now */
  GST_OBJECT_LOCK (pool);
  if (pool->other_pool) {
    gst_object_replace ((GstObject **) & buffer->pool,
        (GstObject *) pool->other_pool);
  }
  GST_OBJECT_UNLOCK (pool);

  gst_mini_object_set_qdata (GST_MINI_OBJECT_CAST (buffer),
      gst_tbm_buffer_data_quark, NULL, NULL);

  GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->free_buffer (bpool,
      buffer);
}

static GstFlowReturn
gst_tbm_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstFlowReturn ret;
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);

  if (pool->port->port_def.eDir == OMX_DirOutput) {
    GstBuffer *buf;

    g_return_val_if_fail (pool->current_buffer_index != -1, GST_FLOW_ERROR);

    buf = g_ptr_array_index (pool->buffers, pool->current_buffer_index);
    g_return_val_if_fail (buf != NULL, GST_FLOW_ERROR);
    *buffer = buf;
    ret = GST_FLOW_OK;

    /* If it's our own memory we have to set the sizes */
    if (!pool->other_pool) {
      GstMemory *mem = gst_buffer_peek_memory (*buffer, 0);

      g_assert (mem
          && g_strcmp0 (mem->allocator->mem_type, GST_OMX_MEMORY_TYPE) == 0);
      mem->size = ((GstOMXMemory *) mem)->buf->omx_buf->nFilledLen;
      mem->offset = ((GstOMXMemory *) mem)->buf->omx_buf->nOffset;
    }
  } else {
    /* Acquire any buffer that is available to be filled by upstream */
    ret =
        GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->acquire_buffer
        (bpool, buffer, params);
  }

  return ret;
}

static void
gst_tbm_buffer_pool_release_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (bpool);
  OMX_ERRORTYPE err;
  GstOMXBuffer *omx_buf;

  g_assert (pool->component && pool->port);

  if (!pool->allocating && !pool->deactivated) {
    omx_buf =
        gst_mini_object_get_qdata (GST_MINI_OBJECT_CAST (buffer),
        gst_omx_buffer_data_quark);
    if (pool->port->port_def.eDir == OMX_DirOutput && !omx_buf->used) {
      /* Release back to the port, can be filled again */
      err = gst_omx_port_release_buffer (pool->port, omx_buf);
      if (err != OMX_ErrorNone) {
        GST_ELEMENT_ERROR (pool->element, LIBRARY, SETTINGS, (NULL),
            ("Failed to relase output buffer to component: %s (0x%08x)",
                gst_omx_error_to_string (err), err));
      }
    } else if (!omx_buf->used) {
      /* TODO: Implement.
       *
       * If not used (i.e. was not passed to the component) this should do
       * the same as EmptyBufferDone.
       * If it is used (i.e. was passed to the component) this should do
       * nothing until EmptyBufferDone.
       *
       * EmptyBufferDone should release the buffer to the pool so it can
       * be allocated again
       *
       * Needs something to call back here in EmptyBufferDone, like keeping
       * a ref on the buffer in GstOMXBuffer until EmptyBufferDone... which
       * would ensure that the buffer is always unused when this is called.
       */
      g_assert_not_reached ();
      GST_BUFFER_POOL_CLASS (gst_tbm_buffer_pool_parent_class)->release_buffer
          (bpool, buffer);
    }
  }
}

static void
gst_tbm_buffer_pool_finalize (GObject * object)
{
  GstTBMBufferPool *pool = GST_TBM_BUFFER_POOL (object);

  if (pool->element)
    gst_object_unref (pool->element);
  pool->element = NULL;

  if (pool->buffers)
    g_ptr_array_unref (pool->buffers);
  pool->buffers = NULL;

  if (pool->other_pool)
    gst_object_unref (pool->other_pool);
  pool->other_pool = NULL;

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  pool->allocator = NULL;

  if (pool->caps)
    gst_caps_unref (pool->caps);
  pool->caps = NULL;

  G_OBJECT_CLASS (gst_tbm_buffer_pool_parent_class)->finalize (object);
}

static void
gst_tbm_buffer_pool_class_init (GstTBMBufferPoolClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *) klass;

  gst_omx_buffer_data_quark = g_quark_from_static_string ("GstOMXBufferData");

  gobject_class->finalize = gst_tbm_buffer_pool_finalize;
  gstbufferpool_class->start = gst_tbm_buffer_pool_start;
  gstbufferpool_class->stop = gst_tbm_buffer_pool_stop;
  gstbufferpool_class->get_options = gst_tbm_buffer_pool_get_options;
  gstbufferpool_class->set_config = gst_tbm_buffer_pool_set_config;
  gstbufferpool_class->alloc_buffer = gst_tbm_buffer_pool_alloc_buffer;
  gstbufferpool_class->free_buffer = gst_tbm_buffer_pool_free_buffer;
  gstbufferpool_class->acquire_buffer = gst_tbm_buffer_pool_acquire_buffer;
  gstbufferpool_class->release_buffer = gst_tbm_buffer_pool_release_buffer;
}

static void
gst_tbm_buffer_pool_init (GstTBMBufferPool * pool)
{
  pool->buffers = g_ptr_array_new ();
  pool->allocator = g_object_new (gst_omx_memory_allocator_get_type (), NULL);
}

static GstBufferPool *
gst_tbm_buffer_pool_new (GstElement * element, GstOMXComponent * component,
    GstOMXPort * port)
{
  GstTBMBufferPool *pool;

  pool = g_object_new (gst_tbm_buffer_pool_get_type (), NULL);
  pool->element = gst_object_ref (element);
  pool->component = component;
  pool->port = port;

  return GST_BUFFER_POOL (pool);
}
#endif


#include "gsttbmbufferpool.h"
#include <gst/video/gstvideofilter.h>

static GstMemory *
gst_mm_memory_allocator_alloc_dummy (GstAllocator * allocator, gsize size,
    GstAllocationParams * params)
{
  g_assert_not_reached ();
  return NULL;
}

static void
gst_mm_memory_allocator_free (GstAllocator * allocator, GstMemory * mem)
{
  GstMMVideoMemory *omem = (GstMMVideoMemory *) mem;

  /* TODO: We need to remember which memories are still used
   * so we can wait until everything is released before allocating
   * new memory
   */

  g_slice_free (GstMMVideoMemory, omem);
}

static gpointer
gst_mm_memory_map (GstMemory * mem, gsize maxsize, GstMapFlags flags)
{
  GstMMVideoMemory *omem = (GstMMVideoMemory *) mem;

  return omem->mm_video_buffer;
}

static void
gst_mm_memory_unmap (GstMemory * mem)
{
}

static GstMemory *
gst_mm_memory_share (GstMemory * mem, gssize offset, gssize size)
{
  g_assert_not_reached ();
  return NULL;
}

GType gst_mm_memory_allocator_get_type (void);
G_DEFINE_TYPE (GstMMVideoMemoryAllocator, gst_mm_memory_allocator,
    GST_TYPE_ALLOCATOR);

#define GST_TYPE_MM_MEMORY_ALLOCATOR   (gst_mm_memory_allocator_get_type())
#define GST_IS_MM_MEMORY_ALLOCATOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_MM_MEMORY_ALLOCATOR))

static void
gst_mm_memory_allocator_class_init (GstMMVideoMemoryAllocatorClass * klass)
{
  GstAllocatorClass *allocator_class;

  allocator_class = (GstAllocatorClass *) klass;

  allocator_class->alloc = gst_mm_memory_allocator_alloc_dummy;
  allocator_class->free = gst_mm_memory_allocator_free;
}

static void
gst_mm_memory_allocator_init (GstMMVideoMemoryAllocator * allocator)
{
  GstAllocator *alloc = GST_ALLOCATOR_CAST (allocator);

  alloc->mem_type = GST_MM_VIDEO_MEMORY_TYPE;
  alloc->mem_map = gst_mm_memory_map;
  alloc->mem_unmap = gst_mm_memory_unmap;
  alloc->mem_share = gst_mm_memory_share;

  /* default copy & is_span */

  GST_OBJECT_FLAG_SET (allocator, GST_ALLOCATOR_FLAG_CUSTOM_ALLOC);
}

static GstMemory *
gst_mm_memory_allocator_alloc (GstAllocator * allocator, GstMemoryFlags flags,
    GstMMBuffer * buf)
{
  GstMMVideoMemory *mem;

  /* FIXME: We don't allow sharing because we need to know
   * when the memory becomes unused and can only then put
   * it back to the pool. Which is done in the pool's release
   * function
   */
  flags |= GST_MEMORY_FLAG_NO_SHARE;

  mem = g_slice_new (GstMMVideoMemory);
  /* the shared memory is always readonly */
  gst_memory_init (GST_MEMORY_CAST (mem), flags, allocator, NULL,
      sizeof(MMVideoBuffer), 1, 0, sizeof(MMVideoBuffer));

  mem->mm_video_buffer = buf;

  return GST_MEMORY_CAST (mem);
}

/* Subclassing GstBufferPool */



GType gst_mm_buffer_pool_get_type (void);

G_DEFINE_TYPE (GstMMBufferPool, gst_mm_buffer_pool, GST_TYPE_BUFFER_POOL);

static gboolean
gst_mm_buffer_pool_start (GstBufferPool * bpool)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);
  return
      GST_BUFFER_POOL_CLASS (gst_mm_buffer_pool_parent_class)->start (bpool);
}

static gboolean
gst_mm_buffer_pool_stop (GstBufferPool * bpool)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);

  if(gst_buffer_pool_is_active (pool) == TRUE)
      return FALSE;
  /* Remove any buffers that are there */
  if(pool->buffers != NULL){
    g_ptr_array_set_size (pool->buffers, 0);
    pool->buffers = NULL;
  }

  if (pool->caps)
    gst_caps_unref (pool->caps);
  pool->caps = NULL;

  pool->add_videometa = FALSE;

  return GST_BUFFER_POOL_CLASS (gst_mm_buffer_pool_parent_class)->stop (bpool);
}

static const gchar **
gst_mm_buffer_pool_get_options (GstBufferPool * bpool)
{
  static const gchar *raw_video_options[] =
      { GST_BUFFER_POOL_OPTION_VIDEO_META, NULL };
  static const gchar *options[] = { NULL };
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);

  return options;
}

static gboolean
gst_mm_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);
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

  return GST_BUFFER_POOL_CLASS (gst_mm_buffer_pool_parent_class)->set_config
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
gst_mm_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);
  GstBuffer *buf = NULL;
  GstMMBuffer *mm_buf = NULL;
  GstMemory *mem;
  void *mm_data = NULL;
  MMVideoBuffer *mm_video_buf = NULL;
  gsize offset[4] = { 0, };
  gint stride[4] = { 0, };
  int width, height;

  mm_buf = (GstMMBuffer*) malloc(sizeof(GstMMBuffer));
  mem = gst_mm_memory_allocator_alloc (pool->allocator, 0, mm_buf);
  buf = gst_buffer_new ();
  buf->pool = pool;
  mem->size = sizeof(GstMMBuffer);
  mem->offset = 0;
  gst_buffer_append_memory (buf, mem);
  offset[0] = offset[1] = 0;
  stride[0] = stride[1] = 0;
  gst_buffer_add_video_meta_full (buf, GST_VIDEO_FRAME_FLAG_NONE,
        GST_VIDEO_INFO_FORMAT (&pool->video_info),
        GST_VIDEO_INFO_WIDTH (&pool->video_info),
        GST_VIDEO_INFO_HEIGHT (&pool->video_info),
        GST_VIDEO_INFO_N_PLANES (&pool->video_info), offset, stride);

  g_ptr_array_add (pool->buffers, buf);
  GST_DEBUG(" buffer:[%p], mm_buffer:[%p], mem:[%p] width:[%d] height:[%d]",buf, mm_buf, mem, GST_VIDEO_INFO_WIDTH (&pool->video_info),GST_VIDEO_INFO_HEIGHT (&pool->video_info));

  gst_mini_object_set_qdata (GST_MINI_OBJECT_CAST (buf),
      gst_mm_buffer_data_quark, mm_buf, NULL);

  /* Allocating second GstMemory for buffer */
    width = GST_VIDEO_INFO_WIDTH (&pool->video_info);
    height = GST_VIDEO_INFO_HEIGHT (&pool->video_info);
    mm_data = g_malloc0(sizeof(MMVideoBuffer));
    mm_video_buf = (MMVideoBuffer*) mm_data;
    mm_video_buf->type = MM_VIDEO_BUFFER_TYPE_TBM_BO;
    mm_video_buf->plane_num = 2;
    /* Setting Y plane size */
    mm_video_buf->size[0] = width * height;
    /* Setting UV plane size */
    mm_video_buf->size[1] = (width * height) >> 1;
    mm_video_buf->handle.bo[0] = tbm_bo_alloc(pool->hTBMBufMgr, mm_video_buf->size[0], TBM_BO_WC);
    mm_video_buf->handle.bo[1] = tbm_bo_alloc(pool->hTBMBufMgr, mm_video_buf->size[1], TBM_BO_WC);

    mm_video_buf->handle.paddr[0] = (tbm_bo_map(mm_video_buf->handle.bo[0], TBM_DEVICE_CPU,TBM_OPTION_WRITE)).ptr;
    mm_video_buf->handle.paddr[1] = (tbm_bo_map(mm_video_buf->handle.bo[1], TBM_DEVICE_CPU,TBM_OPTION_WRITE)).ptr;
    /* Setting stride height & width for Y plane */
    mm_video_buf->stride_height[0] = mm_video_buf->height[0] = height;
    mm_video_buf->stride_width[0] = mm_video_buf->width[0] = width;
    /* Setting stride height & width for UV plane */
    mm_video_buf->stride_height[1] = mm_video_buf->height[1] = height >> 1;
    mm_video_buf->stride_width[1] = mm_video_buf->width[1] = width;

    mem = gst_memory_new_wrapped(0,
        mm_data, sizeof(MMVideoBuffer), 0, sizeof(MMVideoBuffer), mm_data, g_free);
    GST_DEBUG("[%s] Appending 2 memory to gst buffer.",__FUNCTION__);
    gst_buffer_append_memory(buf, mem);

  *buffer = buf;

  pool->current_buffer_index++;

  return GST_FLOW_OK;
}


static void
gst_mm_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  MMVideoBuffer *mm_video_buf = NULL;
  GstMemory *mem = NULL;
  void *mm_data = NULL;
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);

  /* If the buffers belong to another pool, restore them now */
  GST_OBJECT_LOCK (pool);
  if (pool->other_pool) {
    gst_object_replace ((GstObject **) & buffer->pool,
        (GstObject *) pool->other_pool);
  }
  GST_OBJECT_UNLOCK (pool);

  gst_mini_object_set_qdata (GST_MINI_OBJECT_CAST (buffer),
      gst_mm_buffer_data_quark, NULL, NULL);

#ifdef USE_TBM_BUFFER
  GstMapInfo map_info = GST_MAP_INFO_INIT;
  mem = gst_buffer_peek_memory (buffer, 1);
  if (mem != NULL) {
      gst_memory_map(mem, &map_info, GST_MAP_READ);
      mm_data = map_info.data;
      gst_memory_unmap(mem, &map_info);
      mm_video_buf = (MMVideoBuffer*) mm_data;
      if(mm_video_buf != NULL && mm_video_buf->handle.bo[0] != NULL) {
         tbm_bo_unmap(mm_video_buf->handle.bo[0]);
         tbm_bo_unref(mm_video_buf->handle.bo[0]);
      }
      if(mm_video_buf != NULL && mm_video_buf->handle.bo[1] != NULL) {
         tbm_bo_unmap(mm_video_buf->handle.bo[1]);
         tbm_bo_unref(mm_video_buf->handle.bo[1]);
      }
  }
#endif
  GST_BUFFER_POOL_CLASS (gst_mm_buffer_pool_parent_class)->free_buffer (bpool,
      buffer);
}

static GstFlowReturn
gst_mm_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstFlowReturn ret;
  GstMMBuffer *temp = NULL;
  GstMMVideoMemory *temp_video_memory = NULL;
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);

  GstBuffer *buf;
  int i ,n;

  n = pool->buffers->len;
  if(g_queue_is_empty(pool->mm_buffers)) {
      GST_ERROR(" mm_buffers is queue is empty.");
      return GST_FLOW_ERROR;
  }
  temp = g_queue_pop_head(pool->mm_buffers);
  for(i = 0; i < n; i++) {
      temp_video_memory = g_ptr_array_index(pool->buffers,i);
      temp_video_memory->mm_video_buffer = gst_mini_object_get_qdata (GST_MINI_OBJECT_CAST (temp_video_memory),
        gst_mm_buffer_data_quark);
      if(temp_video_memory->mm_video_buffer == temp)
          break;
   }
   if(i == n) {
       GST_ERROR(": mm_buffer is not found in buffers list.");
       return GST_FLOW_ERROR;
   }

   pool->current_buffer_index = i;
   g_return_val_if_fail (pool->current_buffer_index != -1, GST_FLOW_ERROR);
   GST_DEBUG(" Acquiring buffer at index:[%d]",pool->current_buffer_index);
   buf = g_ptr_array_index (pool->buffers, pool->current_buffer_index);
   GST_DEBUG("\ buffer:[%p]",buf);
   g_return_val_if_fail (buf != NULL, GST_FLOW_ERROR);
   *buffer = buf;
   ret = GST_FLOW_OK;

  return ret;
}

static void
gst_mm_buffer_pool_release_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (bpool);
  int err;
  GstMMBuffer *mm_buf;

  if (!pool->allocating && !pool->deactivated) {
    mm_buf =
        gst_mini_object_get_qdata (GST_MINI_OBJECT_CAST (buffer),
        gst_mm_buffer_data_quark);
    if(mm_buf != NULL) {
        g_queue_push_tail(pool->mm_buffers, mm_buf);
    }
  }
}

static void
gst_mm_buffer_pool_finalize (GObject * object)
{
  GstMMBufferPool *pool = GST_MM_BUFFER_POOL (object);

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

  G_OBJECT_CLASS (gst_mm_buffer_pool_parent_class)->finalize (object);
}

static void
gst_mm_buffer_pool_class_init (GstMMBufferPoolClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *) klass;

  gst_mm_buffer_data_quark = g_quark_from_static_string ("GstMMBufferData");

  gobject_class->finalize = gst_mm_buffer_pool_finalize;
  gstbufferpool_class->start = gst_mm_buffer_pool_start;
  gstbufferpool_class->stop = gst_mm_buffer_pool_stop;
  gstbufferpool_class->get_options = gst_mm_buffer_pool_get_options;
  gstbufferpool_class->set_config = gst_mm_buffer_pool_set_config;
  gstbufferpool_class->alloc_buffer = gst_mm_buffer_pool_alloc_buffer;
  gstbufferpool_class->free_buffer = gst_mm_buffer_pool_free_buffer;
  gstbufferpool_class->acquire_buffer = gst_mm_buffer_pool_acquire_buffer;
  gstbufferpool_class->release_buffer = gst_mm_buffer_pool_release_buffer;
}

static void
gst_mm_buffer_pool_init (GstMMBufferPool * pool)
{

  pool->buffers = g_ptr_array_new ();
  pool->allocator = g_object_new (gst_mm_memory_allocator_get_type (), NULL);
  pool->other_pool = NULL;
  pool->mm_buffers = g_queue_new();

#ifdef USE_TBM_BUFFER
  pool->hTBMBufMgr = tbm_bufmgr_init(-1);
  if(pool->hTBMBufMgr == NULL) {
   GST_ERROR ("TBM initialization failed.");
  }
#endif

}

GstBufferPool *
gst_mm_buffer_pool_new (GstElement * trans )
{
  GstMMBufferPool *pool;
  GstVideoFilter *filter = GST_VIDEO_FILTER_CAST (trans);

  pool = g_object_new (gst_mm_buffer_pool_get_type (), NULL);
  pool->video_info = filter->out_info;

  pool->element = gst_object_ref (trans);

  return GST_BUFFER_POOL (pool);
}



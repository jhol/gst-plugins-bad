gsttranscoder_plugin = library('gsttranscode',
  'gsttranscodebin.c',
  'gst-cpu-throttling-clock.c',
  'gsturitranscodebin.c',
  install : true,
  dependencies : [gst_dep, gstpbutils_dep],
  c_args : gst_plugins_bad_args,
  include_directories : [configinc],
  install_dir : plugins_install_dir,
)

pkgconfig.generate(gsttranscoder_plugin, install_dir : plugins_pkgconfig_install_dir)
plugins += [gsttranscoder_plugin]
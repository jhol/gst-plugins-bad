autoconvert2_sources = [
  'gstautoconvert2.c',
  'plugin.c',
]

gstautoconvert2 = library('gstautoconvert2',
  autoconvert2_sources,
  c_args : gst_plugins_bad_args,
  include_directories : [configinc],
  dependencies : [gstbase_dep],
  install : true,
  install_dir : plugins_install_dir,
)
pkgconfig.generate(gstautoconvert2,
  install_dir : plugins_pkgconfig_install_dir)
plugins += [gstautoconvert2]

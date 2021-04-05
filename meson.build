project('meraki', 'c')

src = [
  'measure.c',
  'hash.c',
  'output.c',
  'input.c',
  'meraki.c'
]

libmeraki = both_libraries('meraki', src)
libmeraki_dep = declare_dependency(link_with : libmeraki,
  include_directories : include_directories('.')
)

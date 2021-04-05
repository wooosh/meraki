project('meraki', 'c')

src = [
  'measure.c',
  'output.c',
  'input.c',
  'term.c'
]

inc = include_directories('include')

libmeraki = both_libraries('meraki', src,
  include_directories : inc
)
libmeraki_dep = declare_dependency(link_with : libmeraki,
  include_directories : inc
)

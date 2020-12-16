# See LICENSE for license details.

# Create the include dirs
proc load_incdirs_manifest {incdir_manifest} {
  set fp [open $incdir_manifest r]
  set files [lsearch -not -exact -all -inline [split [read $fp] "\n"] {}]

  close $fp
  return $files

}
 
# Read the specified list of IP files
read_ip [glob -directory $ipdir [file join * {*.xci}]]

# Capture the include directories
set include_directories [load_incdirs_manifest $incdir_manifest]

puts "DEBUG: include_directories = $include_directories"

# Synthesize the design
synth_design -top $top -flatten_hierarchy rebuilt -include_dirs $include_directories

# Checkpoint the current design
write_checkpoint -force [file join $wrkdir post_synth]

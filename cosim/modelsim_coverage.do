set NumericStdNoWarnings 1
set StdArithNoWarnings 1
#
#vcover merge -du DevKitFPGADesign -recursive -out topDesign.ucdb *.ucdb
#rm -f topDesign.ucdb
#rm -f cepMerge.ucdb
#
# Merge top
#
vcover merge -warning 6854 -o cepMerge.ucdb *.ucdb
coverage open cepMerge.ucdb
#coverage read -dataset cepMerge cepMerge.ucdb

#
# Exclude
#
coverage exclude  -du {*tl_master*}  
coverage exclude  -du {*RocketTile_beh*}
coverage exclude  -du {plusarg_reader}
coverage exclude  -du {*TLMonitor*} 
#coverage exclude -du {plusarg_reader} -togglenode {myplus[0:31]}
#coverage exclude -du {plusarg_reader*} -togglenode {out[0:31]}
coverage exclude  -du {*} -togglenode {_RAND_*}
coverage exclude  -du {*} -togglenode {initvar}
# pma checker not use at all
coverage exclude  -scope {/cep_tb/fpga/topDesign/topMod/tile/dcache/pma_checker} -recursive -allfalse
coverage exclude  -scope {/cep_tb/fpga/topDesign/topMod/tile_1/dcache/pma_checker} -recursive -allfalse
coverage exclude  -scope {/cep_tb/fpga/topDesign/topMod/tile_2/dcache/pma_checker} -recursive -allfalse
coverage exclude  -scope {/cep_tb/fpga/topDesign/topMod/tile_3/dcache/pma_checker} -recursive -allfalse

#
# Save and extract
# 
coverage save -codeAll cepMerge.ucdb
vcover merge -du DevKitFPGADesign -recursive -out topDesign.ucdb cepMerge.ucdb

#
# Report
#
coverage open topDesign.ucdb
coverage report -html -output covhtmlreport -details -assert -directive -cvg -code bcefst -threshL 50 -threshH 90 

#
# Done
#
quit -f


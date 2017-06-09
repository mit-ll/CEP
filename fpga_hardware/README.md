To add a core to the SoC as a slave:

Add files that require compilation to the top of build_support/orpsoc_top.prj

If the new core requires a header file, update "-vlgincdir{}" in build_support/orpsoc_top.xst

Update the Makefile:
	Create an alias to the core's directory
	Use the created alias to update "IVERILOG_SRCS"
    Add the alias to the orpsoc.vvp target
	Add the core to the "SYNTHESIS TARGETS" dependency list

Add the core to the SoC's Wishbone Bus:
    Add a define that will control whether the core is included in the SoC to cores/top/orpsoc-defines.v
	Add the core's information to orpsoc-params.v
        Use the Ethernet core as an example
        Make sure to use an available address prefix for "XX_wb_adr"
        Refer to the number of address actually used by the new core's bus interface to determine the address width
        All other widths are the default of 32 bits
	Update the configuration of the appropriate bus(es) (i.e., instruction, data, byte-wide) in cores/top/orpsoc-params.v
        Increment the number of slaves
        Insert the new core before any byte-wide or RAM/ROM slave
    Modify the appropriate bus aribiter  (i.e., cores/arbiter/arbiter_XX.v) if you had to modify the number of slaves in the previous step or used a unique address for the new core
        Add a new set of Wishbone signals to the arbiter's interface
            It is easier to change the names in the comments than the signal numbers
        Increment the number of slaves
        Add a "slaveX_adr" and a "slaveX_addr_width" parameter
            The values you set here aren't important because they will be overriden by the module that instantiates the aribter
        Add a save select entry from the new core and update the array indexing to match the new slave numbering
        Copy and renumber a "Slave X input" wire connection block
        Update the master mux signals to include the new slave
	In cores/top/orpsoc_top.v
        Add any pin connections to the interface of this module
        Create a set of bus slave wires to connect the new core and the appropriate bus arbiter
        Connect the new core to the appropriate aribter using the wires created in the previous step
            Make sure to number of all cores for this arbiter is updated as well
        Update that bus aribter's defparams both in terms of numbering and adding defparams for the new core's "adr" and "add_width" parameters
        Use conditional compilation statements and the define created earlier to conditionally instantiate the new core
            Drive slave outputs to 0's if new core not defined
        If the new core has an interrupt line, using the define, conditionally connect it to the processor's interrupt controller

If the core requires pin I/O connections:
	Bring the signals up through the hierarchy
	Add the pinout information to backend/ml509.ucf

 If you want to simulate the SoC with the new core, you need to add it to the based ModelSim project file (a2.mpf).
	Increase "Project_Files_Count" to account for the additional files
	Using one of the existing "Project_File_XX"'s as a template add any files that require compilation
		Make sure to specify any required header files


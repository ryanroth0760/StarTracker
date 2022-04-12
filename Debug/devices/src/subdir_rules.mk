################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
devices/src/%.obj: ../devices/src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-msp430_20.2.2.LTS/bin/cl430" -vmspx --code_model=large --data_model=restricted --near_data=none --use_hw_mpy=F5 --include_path="C:/Users/troth/workspace_v10/StarTracker/devices/inc" --include_path="C:/Users/troth/workspace_v10/StarTracker/devices/src" --include_path="C:/ti/ccs1020/ccs/ccs_base/msp430/include" --include_path="C:/Users/troth/workspace_v10/StarTracker" --include_path="C:/Users/troth/workspace_v10/StarTracker/driverlib/MSP430F5xx_6xx" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-msp430_20.2.2.LTS/include" --advice:power="all" --define=__MSP430F5529__ -g --c11 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="devices/src/$(basename $(<F)).d_raw" --obj_directory="devices/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



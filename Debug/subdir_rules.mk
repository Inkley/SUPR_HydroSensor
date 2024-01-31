################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --include_path="/Applications/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --include_path="/Applications/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include/sys" --include_path="/Users/Tyler/ti/tivaware_c_series_2_1_4_178" --preinclude="/Applications/ti/ccs1240/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include/sys/_timeval.h" --advice:power="all" --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 -g --gcc --printf_support=full --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



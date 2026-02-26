#ifndef SHARED
#define SHARED

#include "entry.h"
#include "verilogcode.h"
#include "verilogjjparser.h"
#include "sharedstate.h"

static std::string  ruleContext[609]  =   {"RuleInclude_path" , "RuleLibrary_description"  , "RuleSource_code" , 
    "RuleDescription" , "RuleModule_nonansi_header" , "RuleModule_declaration" , 
    "RuleModule_keyword" , "RuleInterface_declaration" , "RuleInterface_nonansi_header" , 
    "RuleProgram_declaration" , "RuleProgram_nonansi_header"  , "RuleChecker_declaration"   , 
    "RuleClass_declaration"  , "RuleInterface_class_type"  , "RuleInterface_class_declaration"  , 
    "RuleInterface_class_item"  , "RuleInterface_class_method"  , "RulePackage_declaration"  , 
    "RuleTimeunits_declaration"  , "RuleParameter_port_list"  , "RuleParameter_port_declaration" , 
    "RuleParameter_declaration"  , "RuleList_of_ports" , "RuleList_of_port_declarations" , 
    "RuleParam_assignment" , "RuleList_of_param_assignments" , "RulePort_declaration" , 
    "RulePort" , "RulePort_expression" , "RulePort_reference" , "RulePort_direction" , 
    "RuleNet_port_header"  , "RuleVariable_port_header" , "RuleInterface_port_header" , 
    "RuleAnsi_port_declaration" , "RuleElaboration_system_task" , "RuleFinish_number" , 
    "RuleModule_common_item" , "RuleModule_item" , "RuleModule_or_generate_item" , 
    "RuleModule_or_generate_item_declaration" , "RuleNon_port_module_item"  , 
    "RuleParameter_override" , "RuleBind_directive" , "RuleBind_target_instance" , 
    "RuleBind_target_instance_list" , "RuleBind_instantiation" , "RuleConfig_declaration" , 
    "RuleDesign_statement" , "RuleConfig_rule_statement" , "RuleDefault_clause" , 
    "RuleInst_clause"  , "RuleInst_name" , "RuleCell_clause" , "RuleLiblist_clause" , 
    "RuleUse_clause" , "RuleInterface_or_generate_item" , "RuleExtern_tf_declaration" , 
    "RuleInterface_item" , "RuleNon_port_interface_item" , "RuleProgram_item" , 
    "RuleNon_port_program_item"  , "RuleProgram_generate_item" , "RuleChecker_port_list" , 
    "RuleChecker_port_item" , "RuleChecker_port_direction" , "RuleChecker_or_generate_item" , 
    "RuleChecker_or_generate_item_declaration" , "RuleChecker_generate_item" , 
    "RuleClass_item" , "RuleClass_property" , "RuleClass_method"  , 
    "RuleClass_constructor_prototype" , "RuleClass_constraint" , "RuleClass_item_qualifier" , 
    "RuleProperty_qualifier" , "RuleRandom_qualifier" , "RuleMethod_qualifier" , 
    "RuleMethod_prototype" , "RuleClass_constructor_declaration" , "RuleConstraint_declaration" , 
    "RuleConstraint_block"  , "RuleConstraint_block_item" , "RuleSolve_before_list" , 
    "RuleConstraint_primary" , "RuleConstraint_expression" , "RuleUniqueness_constraint" , 
    "RuleConstraint_set" , "RuleDist_list" , "RuleDist_item" , "RuleDist_weight" , 
    "RuleConstraint_prototype"  , "RuleConstraint_prototype_qualifier" , 
    "RuleExtern_constraint_declaration" , "RuleIdentifier_list" , "RulePackage_item" , 
    "RulePackage_or_generate_item_declaration" , "RuleAnonymous_program" , 
    "RuleAnonymous_program_item" , "RuleLocal_parameter_declaration" , 
    "RuleSpecparam_declaration"  , "RuleInout_declaration"   , "RuleInput_declaration"  , 
    "RuleOutput_declaration"  , "RuleInterface_port_declaration"  , 
    "RuleRef_declaration"  , "RuleUuu"  , "RuleData_declaration"  , 
    "RulePackage_import_declaration"  , "RulePackage_import_item"  , 
    "RulePackage_export_declaration"   , "RuleGenvar_declaration"    , 
    "RuleNet_declaration"   , "RuleType_declaration"   , "RuleNet_type_declaration"   , 
    "RuleLifetime"   , "RuleCasting_type"   , "RuleData_type"   , "RuleData_type_or_implicit"   , 
    "RuleData_type_or_implicit "   , "RuleImplicit_data_type"  , "RuleEnum_base_type"   , 
    "RuleEnum_name_declaration"  , "RuleClass_scope"  , "RuleClass_type "  , 
    "RuleClass_type"  , "RuleInteger_type"  , "RuleInteger_atom_type"  , 
    "RuleInteger_vector_type"  , "RuleNon_integer_type"  , "RuleNet_type"  , 
    "RuleNet_port_type"   , "RuleVariable_port_type"  , "RuleVar_data_type"  , 
    "RuleSigning"  , "RuleSimple_type"  , "RuleStruct_union_member"  , 
    "RuleData_type_or_void"  , "RuleStruct_union"  , "RuleType_reference"  , 
    "RuleDrive_strength"  , "RuleStrength"   , "RuleStrength "  , 
    "RuleCharge_strength"  , "RuleDelay"  , "RuleDelay"  , "RuleDelay_value"  , 
    "RuleList_of_defparam_assignments"  , "RuleList_of_genvar_identifiers"  , 
    "RuleList_of_interface_identifiers"  , "RuleList_of_net_decl_assignments"  , 
    "RuleList_of_port_identifiers"   , "RuleList_of_udp_port_identifiers"  , 
    "RuleList_of_specparam_assignments"  , "RuleList_of_tf_variable_identifiers"  , 
    "RuleList_of_type_assignments"  , "RuleList_of_variable_decl_assignments"  , 
    "RuleList_of_variable_identifiers"  , "RuleList_of_variable_port_identifiers"  , 
    "RulePort_id"  , "RuleDefparam_assignment"  , "RuleNet_decl_assignment"   , 
    "RuleSpecparam_assignment"  , "RuleType_assignment"  , "RulePulse_control_specparam"  , 
    "RuleError_limit_value"  , "RuleReject_limit_value"  , "RuleLimit_value"  , 
    "RuleVariable_decl_assignment"  , "RuleClass_new"  , "RuleDynamic_array_new"  , 
    "RuleUnpacked_dimension"   , "RulePacked_dimension"  , "RuleAssociative_dimension"  , 
    "RuleVariable_dimension"  , "RuleQueue_dimension"  , "RuleUnsized_dimension"  , 
    "RuleFunction_data_type_or_implicit"  , "RuleFunction_declaration"  , 
    "RuleFunction_body_declaration"  , "RuleFunction_dec"  , "RuleFunction_prototype"   , 
    "RuleDpi_import_export"  , "RuleDpi_spec_string"  , "RuleDpi_function_import_property"  , 
    "RuleDpi_task_import_property"  , "RuleDpi_function_proto"  , "RuleDpi_task_proto"  , 
    "RuleTask_declaration"  , "RuleTask_body_declaration"  , "RuleTask_decl"  , 
    "RuleTf_item_declaration"   , "RuleTf_port_list"  , "RuleTf_port_item"  , 
    "RuleTf_port_direction"  , "RuleTf_port_declaration"  , "RuleTask_prototype"  , 
    "RuleBlock_item_declaration"  , "RuleOverload_declaration"  , "RuleOverload_operator"  , 
    "RuleOverload_proto_formals" , "RuleModport_declaration"  , "RuleModport_item" , 
    "RuleModport_ports_declaration" , "RuleModport_clocking_declaration" , 
    "RuleModport_simple_ports_declaration" , "RuleModport_simple_port" , 
    "RuleModport_tf_ports_declaration" , "RuleModport_tf_port" , "RuleImport_export" , 
    "RuleConcurrent_assertion_item"  , "RuleConcurrent_assertion_statement"   , 
    "RuleAssert_property_statement"  , "RuleAssume_property_statement"  , 
    "RuleCover_property_statement"  , "RuleExpect_property_statement"  , 
    "RuleCover_sequence_statement"  , "RuleRestrict_property_statement"  , 
    "RuleProperty_instance"  , "RuleProperty_list_of_arguments"  , "RuleProperty_actual_arg" , 
    "RuleAssertion_item_declaration"  , "RuleProperty_declaration" , 
    "RuleProperty_port_list" , "RuleProperty_port_item" , "RuleProperty_lvar_port_direction" , 
    "RuleProperty_formal_type" , "RuleProperty_spec" , "RuleProperty_expr" , 
    "RuleProperty_case_item" , "RuleSequence_declaration" , "RuleSequence_port_list"  , 
    "RuleSequence_port_item" , "RuleSequence_lvar_port_direction" , 
    "RuleSequence_formal_type" , "RuleSequence_expr" , "RuleCycle_delay_range" , 
    "RuleSequence_method_call" , "RuleSequence_match_item" , "RuleSequence_instance" , 
    "RuleSequence_list_of_arguments" , "RuleSequence_actual_arg"  , 
    "RuleBoolean_abbrev" , "RuleSequence_abbrev" , "RuleConsecutive_repetition" , 
    "RuleNon_consecutive_repetition" , "RuleGoto_repetition" , "RuleConst_or_range_expression" , 
    "RuleCycle_delay_const_range_expression" , "RuleExpression_or_dist" , 
    "RuleAssertion_variable_declaration" , "RuleLet_declaration"  , 
    "RuleLet_identifier" , "RuleLet_port_list" , "RuleLet_port_item" , 
    "RuleLet_formal_type" , "RuleLet_expression" , "RuleLet_list_of_arguments" , 
    "RuleLet_actual_arg" , "RuleCovergroup_declaration" , "RuleCoverage_spec_or_option" , 
    "RuleCoverage_option"  , "RuleCoverage_spec" , "RuleCoverage_event" , 
    "RuleBlock_event_expression" , "RuleHierarchical_btf_identifier" , 
    "RuleCover_point" , "RuleBins_or_empty" , "RuleBins_or_options" , 
    "RuleBins_keyword" , "RuleTrans_list" , "RuleTrans_set"  , "RuleTrans_range_list" , 
    "RuleTrans_item" , "RuleRepeat_range" , "RuleCover_cross" , 
    "RuleList_of_cross_items" , "RuleCross_item" , "RuleCross_body" , 
    "RuleCross_body_item" , "RuleBins_selection_or_option" , "RuleBins_selection"  , 
    "RuleSelect_expression" , "RuleSelect_expression" , "RuleSelect_condition" , 
    "RuleBins_expression" , "RuleCovergroup_range_list" , "RuleCovergroup_value_range" , 
    "RuleWith_covergroup_expression" , "RuleSet_covergroup_expression" , 
    "RuleInteger_covergroup_expression" , "RuleCross_set_expression"  , 
    "RuleCovergroup_expression" , "RuleGate_instantiation" , "RuleCmos_switch_instance" , 
    "RuleEnable_gate_instance" , "RuleMos_switch_instance" , "RuleN_input_gate_instance" , 
    "RuleN_output_gate_instance" , "RulePass_switch_instance" , "RulePass_enable_switch_instance" , 
    "RulePull_gate_instance"  , "RulePulldown_strength" , "RulePullup_strength" , 
    "RuleEnable_terminal" , "RuleInout_terminal" , "RuleInput_terminal" , 
    "RuleNcontrol_terminal" , "RuleOutput_terminal" , "RulePcontrol_terminal" , 
    "RuleCmos_switchtype"  , "RuleEnable_gatetype"   , "RuleMos_switchtype"  , 
    "RuleN_input_gatetype"  , "RuleN_output_gatetype"  , "RulePass_en_switchtype"  , 
    "RulePass_switchtype"  , "RuleModule_instantiation"  , "RuleParameter_value_assignment"  , 
    "RuleList_of_parameter_assignments"  , "RuleOrdered_parameter_assignment" , 
    "RuleNamed_parameter_assignment"  , "RuleHierarchical_instance" , 
    "RuleName_of_instance" , "RuleList_of_port_connections" , "RuleOrdered_port_connection" , 
    "RuleNamed_port_connection" , "RuleInterface_instantiation" , 
    "RuleChecker_instantiation" , "RuleList_of_checker_port_connections" , 
    "RuleOrdered_checker_port_connection" , "RuleNamed_checker_port_connection"  , 
    "RuleProgram_instantiation" , "RuleGenerate_region" , "RuleLoop_generate_construct" , 
    "RuleGenvar_initialization" , "RuleGenvar_iteration" , "RuleConditional_generate_construct" , 
    "RuleIf_generate_construct" , "RuleCase_generate_construct" , 
    "RuleCase_generate_item" , "RuleGenerate_block"  , "RuleGenerate_item" , 
    "RuleContinuous_assign" , "RuleList_of_net_assignments" , "RuleList_of_variable_assignments" , 
    "RuleNet_alias" , "RuleNet_assignment" , "RuleInitial_construct" , 
    "RuleAlways_construct" , "RuleAlways_keyword" , "RuleFinal_construct"  , 
    "RuleBlocking_assignment" , "RuleOperator_assignment" , "RuleAssignment_operator" , 
    "RuleNonblocking_assignment" , "RuleProcedural_continuous_assignment" , 
    "RuleVariable_assignment" , "RuleAction_block" , "RuleSeq_block" , 
    "RulePar_block" , "RuleJoin_keyword"  , "RuleStatement_or_null" , 
    "RuleStatement" , "RuleStatement_item" , "RuleFunction_statement" , 
    "RuleFunction_statement_or_null" , "RuleVariable_identifier_list" , 
    "RuleProcedural_timing_control_statement" , "RuleDelay_or_event_control" , 
    "RuleDelay_control" , "RulePound_delay_value"  , "RuleEvent_control" , 
    "RuleEvent_expression" , "RuleProcedural_timing_control" , "RuleJump_statement" , 
    "RuleWait_statement" , "RuleEvent_trigger" , "RuleDisable_statement" , 
    "RuleConditional_statement" , "RuleUnique_priority" , "RuleCond_predicate"  , 
    "RuleExpression_or_cond_pattern" , "RuleCond_pattern" , "RuleCase_statement" , 
    "RuleCcase" , "RuleCase_keyword" , "RuleCase_expression" , 
    "RuleCase_item" , "RuleCase_pattern_item" , "RuleCase_inside_item" , 
    "RuleCase_item_expression"  , "RuleRandcase_statement" , "RuleRandcase_item" , 
    "RuleOpen_range_list" , "RuleOpen_value_range" , "RulePattern" , 
    "RuleAssignment_pattern" , "RuleStructure_pattern_key" , "RuleArray_pattern_key" , 
    "RuleAssignment_pattern_key" , "RuleAssignment_pattern_expression"  , 
    "RuleAssignment_pattern_expression_type" , "RuleConstant_assignment_pattern_expression" , 
    "RuleAssignment_pattern_net_lvalue" , "RuleAssignment_pattern_variable_lvalue" , 
    "RuleLoop_statement" , "RuleFor_initialization" , "RuleFor_variable_declaration" , 
    "RuleFor_step" , "RuleFor_step_assignment"  , "RuleLoop_variables"   , 
    "RuleSubroutine_call_statement"  , "RuleAssertion_item"  , "RuleDeferred_immediate_assertion_item"  , 
    "RuleProcedural_assertion_statement"  , "RuleImmediate_assertion_statement"  , 
    "RuleSimple_immediate_assertion_statement"  , "RuleSimple_immediate_assert_statement"  , 
    "RuleSimple_immediate_assume_statement"  , "RuleSimple_immediate_cover_statement" , 
    "RuleDeferred_immediate_assertion_statement"  , "RuleDeferred_immediate_assert_statement" , 
    "RuleDeferred_immediate_assume_statement" , "RuleDeferred_immediate_cover_statement" , 
    "RuleClocking_declaration" , "RuleClocking_event" , "RuleClocking_item" , 
    "RuleDefault_skew" , "RuleClocking_direction" , "RuleList_of_clocking_decl_assign" , 
    "RuleClocking_decl_assign"  , "RuleClocking_skew" , "RuleClocking_drive" , 
    "RuleCycle_delay" , "RuleClockvar" , "RuleClockvar_expression" , 
    "RuleRandsequence_statement" , "RuleProduction" , "RuleRs_rule" , 
    "RuleRs_production_list" , "RuleWeight_specification"  , "RuleRs_code_block" , 
    "RuleRs_prod" , "RuleProduction_item" , "RuleRs_if_else" , 
    "RuleRs_repeat" , "RuleRs_case" , "RuleRs_case_item" , "RuleSpecify_block" , 
    "RuleSpecify_item" , "RulePulsestyle_declaration"  , "RuleShowcancelled_declaration" , 
    "RulePath_declaration" , "RuleSimple_path_declaration" , "RuleEdge_sensitive_path_declaration" , 
    "RuleParallel_path_description" , "RuleParallel_edge_sensitive_path_description" , 
    "RuleFull_path_description" , "RuleFull_edge_sensitive_path_description" , 
    "RuleList_of_path_inputs" , "RuleList_of_path_outputs"  , "RuleSpecify_input_terminal_descriptor" , 
    "RuleSpecify_output_terminal_descriptor" , "RuleInput_identifier" , 
    "RuleOutput_identifier" , "RulePath_delay_value" , "RuleList_of_path_delay_expressions" , 
    "RulePath_delay_expression" , "RuleData_source_expression" , "RuleEdge_identifier" , 
    "RuleState_dependent_path_declaration"  , "RulePolarity_operator" , 
    "RuleSystem_timing_check" , "RuleSetup_timing_check" , "RuleHold_timing_check" , 
    "RuleSetuphold_timing_check" , "RuleRecovery_timing_check" , "RuleRemoval_timing_check" , 
    "RuleRecrem_timing_check" , "RuleSkew_timing_check" , "RuleTimeskew_timing_check"  , 
    "RuleFullskew_timing_check" , "RulePeriod_timing_check" , "RuleWidth_timing_check" , 
    "RuleNochange_timing_check" , "RuleTimecheck_condition" , "RuleControlled_reference_event" , 
    "RuleData_event" , "RuleDelayed_data" , "RuleDelayed_reference" , 
    "RuleEnd_edge_offset"  , "RuleEvent_based_flag" , "RuleNotifier" , 
    "RuleReference_event" , "RuleRemain_active_flag" , "RuleTimestamp_condition" , 
    "RuleStart_edge_offset" , "RuleThreshold" , "RuleTiming_check_limit" , 
    "RuleTiming_check_event" , "RuleControlled_timing_check_event"  , 
    "RuleTiming_check_event_control" , "RuleSpecify_terminal_descriptor" , 
    "RuleEdge_control_specifier" , "RuleEdge_descriptor" , "RuleTiming_check_condition" , 
    "RuleScalar_timing_check_condition" , "RuleScalar_constant" , 
    "RuleConcatenation" , "RuleConstant_concatenation"  , "RuleConstant_multiple_concatenation"   , 
    "RuleModule_path_concatenation"  , "RuleModule_path_multiple_concatenation"  , 
    "RuleMultiple_concatenation"  , "RuleStreaming_concatenation"  , 
    "RuleStream_operator"  , "RuleSlice_size"  , "RuleStream_concatenation"  , 
    "RuleStream_expression"  , "RuleArray_range_expression" , "RuleEmpty_queue"  , 
    "RuleTf_call" , "RuleMethod_call" , "RuleSystem_tf_call" , 
    "RuleSubroutine_call" , "RuleArgument" , "RuleList_of_arguments" , 
    "RuleArray_manipulation_call" , "RuleRandomize_call" , "RuleMethod_name" , 
    "RuleArray_method_name"  , "RuleInc_or_dec_expression" , "RuleConstant_expression" , 
    "RuleConstant_mintypmax_expression" , "RuleConstant_param_expression" , 
    "RuleParam_expression" , "RuleConstant_range_expression" , "RuleConstant_part_select_range" , 
    "RuleConstant_range" , "RuleConstant_indexed_range" , "RuleExpression"  , 
    "RuleConditional_expression" , "RuleTagged_union_expression" , 
    "RuleValue_range" , "RuleMintypmax_expression" , "RuleModule_path_conditional_expression" , 
    "RuleModule_path_expression" , "RuleModule_path_mintypmax_expression" , 
    "RulePart_select_range" , "RuleIndexed_range" , "RuleGenvar_expression"  , 
    "RuleConstant_primary" , "RuleModule_path_primary" , "RulePrimary" , 
    "RuleClass_qualifier" , "RuleRange_expression" , "RulePrimary_literal" , 
    "RuleTime_literal" , "RuleTime_unit" , "RuleImplicit_class_handle" , 
    "RuleBit_select"  , "RuleSelect" , "RuleNonrange_select" , "RuleConstant_bit_select" , 
    "RuleConstant_select" , "RuleConstant_cast" , "RuleConstant_let_expression" , 
    "RuleCast" , "RuleNet_lvalue" , "RuleVariable_lvalue" , "RuleNonrange_variable_lvalue"  , 
    "RuleUnary_operator" , "RuleBinary_operator" , "RuleInc_or_dec_operator" , 
    "RuleUnary_module_path_operator" , "RuleBinary_module_path_operator" , 
    "RuleNumber" , "RulePackage_scope" , "RulePsid" , "RuleHierid" , 
    "RuleSs"  , "RuleUdp_nonansi_declaration" , "RuleUdp_ansi_declaration" , 
    "RuleUdp_declaration" , "RuleUdp_port_list" , "RuleUdp_declaration_port_list" , 
    "RuleUdp_port_declaration" , "RuleUdp_output_declaration" , "RuleUdp_input_declaration" , 
    "RuleUdp_reg_declaration" , "RuleUdp_body"  , "RuleCombinational_body" , 
    "RuleCombinational_entry" , "RuleSequential_body" , "RuleUdp_initial_statement" , 
    "RuleSequential_entry" , "RuleSeq_input_list" , "RuleLevel_input_list" , 
    "RuleEdge_input_list" , "RuleEdge_indicator" , "RuleCurrent_state"  , 
    "RuleNext_state" , "RuleInit_val" , "RuleLevel_symbol" , "RuleOutput_symbol" , 
    "RuleEdge_symbol" , "RuleUdp_instantiation" , "RuleUdp_instance" }; 
 



class DefMacro
{
  public:
    int                                beginLine;
    int                                endLine;
    int                                column;
    int                                arg;
    std::string                        mName;
    std::string                        mDefinition;
    std::string                        fi;
    // defined macro (..,...,)
    std::list<std::string>             li;
    // macros
    std::vector<std::string>           lm;
    std::map<std::string, std::string> hm;

    ~DefMacro() { /*  std::cout << "\ndelete macro:" << mName << "\n"; */ }
    DefMacro() {}

    void        copy(DefMacro *that);
    void        setFileName(std::string f);
    void        setMacroList(std::list<std::string> &li);
    std::string getMacro(int i);
    std::string getSelfDefiniton(std::string &s);
    int         findParam(std::string key);
    std::string getMacroParam(std::string &p);
    void        debug();
};

//typedef std::map<std::string, std::shared_ptr<DefMacro>> DefVec;

template <class T>
class ConcurrentMacroMap
{
  public:
    T *get(const std::string &key)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (auto search = m_map.find(key); search != m_map.end())
        return search->second.get();

      return nullptr;
    }

    void write(const std::string &key, std::shared_ptr<T> df)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_map.insert({ key, df });
    }

    size_t size()
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_map.size();
    }

    bool erase(const std::string &key)
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (auto it = m_map.find(key); it != m_map.end())
      {
        m_map.erase(it);
        return true;
      }

      return false;
    }

    void clear()
    {
      // std::lock_guard<std::mutex> lock(m_mutex);
      m_map.clear();
    }

    //DefVec getMacros() { return m_map; }

    // only for debugging


    void writeMacros()
    {
      std::ofstream outf("prepmacros.txt", std::ofstream::out | std::ofstream::binary);

      TextStream    t(&outf);
      for (const auto &[name, df] : m_map)
      {
        t << "----------------------------------------";
        t << "\n";
        t << df->mName;
        t << " at line: ";
        t << df->beginLine;
        t << "\n";
        t << "map contains:\n";
        for (auto it = df->hm.begin(); it != df->hm.end(); ++it)
        {
          t << it->first;
          t << " => ";
          t << it->second;
          t << '\n';
        }

        for (std::string s : df->li)
        {
          t << "<";
          t << s;
          t << ">\n";
        }

        t << "---------------------------------------\n";
        t << df->mDefinition;
        t << "\n";
        t << "--------------------------------------\n";
      }
      t << "\n\n\nfound: ";
      t << m_map.size();
      t << " macros\n";
    }
  private:
    // DefVec     m_map;
    std::map<std::string, std::shared_ptr<T>> m_map;
    std::mutex                                m_mutex;
};

class IncludeList
{
  private:
    std::mutex            m_mutex;
    std::set<std::string> m_set;
  public:
    void insert(std::string &item)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_set.emplace(item);
    }

    std::string find(std::string &str)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for (std::string path : m_set)
      {
        if (QCString(path).endsWith(str.c_str()))
          return path;
      }
      return "";
    }
};

struct SharedState
{
    std::shared_ptr<Entry>              current_root;
    std::shared_ptr<Entry>              tempEntry;
    std::shared_ptr<Entry>              lastEntity;
    std::shared_ptr<Entry>              lastCompound;
    std::shared_ptr<Entry>              current;
    std::string                         currName;
    std::string                         fileName;
    std::vector<std::shared_ptr<Entry>> shvec;
    VerilogCodeParser                  *m_Code = 0;
    ConcurrentMacroMap<DefMacro>        currMap;
    StringSet                           incPath;
    DefineList                          defList;
    FileDef                            *fdd;
  
    std::map<QCString, std::string>     includePath;
    bool                                isProto = false;
    bool                                g_init  = false;

    void  updateClassEntry()
    {
      if (shvec.empty())
      {
        lastEntity = NULL;
      }
      else
      {
        lastEntity = shvec.back();
      }
    }

    void insertClassEntry(std::shared_ptr<Entry> e)
    {
     QCString qz=getClassName();
     Entry* en=e.get();
     
     if(!qz.isEmpty())
     {
      qz.append("::");
      qz.append(en->name);
      en->name=qz;
     }   
    //  std::cout<<"\nInsert Class : "<<en->name<<" at line: "<<en->startLine<<"\n";
     
      shvec.emplace_back(e);
      updateClassEntry();
    }

    void delClassEntry()
    {
      if (!shvec.empty())
      { 
        shvec.pop_back();
        updateClassEntry();
      }
    }

 QCString getClassName()
  {

    QCString q;

    if (shvec.empty())
    {
      return q;
    }

    q=shvec.back().get()->name;

     return q;
  }

};

#endif
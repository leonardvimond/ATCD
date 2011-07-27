// -*- C++ -*-
// $Id$

//=============================================================================
/**
* @file  SA_WorkingPlan.cpp
*
* This file contains the implementation of the SA_WorkingPlan concrete class,
* which implements a WorkingPlan that uses precedence graphs for scheduling.
*
* @author  John S. Kinnebrew <john.s.kinnebrew@vanderbilt.edu>
*/
//=============================================================================

#include "SA_POP_Types.h"
#include "SA_WorkingPlan.h"
#include "Planner.h"
#include "PlanCommands.h"
#include "SA_PlanCommands.h"

#include <stack>
#include <fstream>

using namespace SA_POP;

// Constructor.
SA_WorkingPlan::SA_WorkingPlan (SA_POP::Planner *planner)
: WorkingPlan (planner),
has_cmds_ (false),
next_inst_id_ (1),
add_task_cmd_ (0),
assoc_impl_cmd_ (0),
resolve_threat_cmd_ (0),
resolve_sched_cmd_ (0),
adj_min_times_cmd_ (0),
adj_max_times_cmd_ (0),
add_threats_cmd_ (0)
{
  // Initialize variables.
  this->reset_plan ();

  // Create template commands.
	this->add_task_cmd_ = new SA_AddTaskCmd (this);
	this->assoc_impl_cmd_ = new SA_AssocTaskImplCmd (this);
	this->resolve_threat_cmd_ = new SA_ResolveCLThreatCmd (this);
	this->resolve_sched_cmd_ = new SA_ResolveSchedOrderCmd (this);
	this->adj_min_times_cmd_ = new SA_AdjustMinTimesCmd (this);
	this->adj_max_times_cmd_ = new SA_AdjustMaxTimesCmd (this);
};

// Destructor.
SA_WorkingPlan::~SA_WorkingPlan (void)
{
	// Nothing to do.
};

//Reset Plan.

void SA_WorkingPlan::reset_plan ()
{
  // Init simple variables.
  this->next_inst_id_ = 1;

  // Clear sets, maps, & structs.
	this->task_insts_.clear ();
	this->task_impls_.clear ();
	this->causal_links_.clear ();
	this->reused_insts_.clear();
	this->sched_links_.clear();
	this->precedence_graph_.clear();
	this->init_start.clear();
	this->init_end.clear();
  this->goal_.clear ();
	this->plan_.clear ();

  // Initialize precedence graph.
	PrecedenceSet temp;
	this->precedence_graph_.insert(std::make_pair(BEFORE,temp));
	this->precedence_graph_.insert(std::make_pair(AFTER,temp));
	this->precedence_graph_.insert(std::make_pair(SIMUL,temp));
	this->precedence_graph_.insert(std::make_pair(UNRANKED,temp));

	this->init_start.insert(std::make_pair(-6, (TimeWindow)std::make_pair(NULL_TIME, NULL_TIME)));
	this->init_end.insert(std::make_pair(-6, (TimeWindow)std::make_pair(NULL_TIME, NULL_TIME)));

	for(int i = 0; i < 2; i++){
		this->init_start.insert(std::make_pair(i,(TimeWindow)std::make_pair(NULL_TIME,NULL_TIME)));
		this->init_end.insert(std::make_pair(i,(TimeWindow)std::make_pair(NULL_TIME,NULL_TIME)));
	}

	this->no_condition.id = -4;
	this->no_link.cond.id = -4;
	this->no_link.first = -4;
	this->no_link.second = -4;

	this->task_impls_.insert(std::pair<TaskInstID, TaskImplID>(INIT_TASK_INST_ID, INIT_TASK_IMPL_ID));
	this->task_insts_.insert(std::pair<TaskInstID, TaskID>(INIT_TASK_INST_ID, INIT_TASK_ID));
}

// Set goal.
void SA_WorkingPlan::set_goal (const SA_POP::Goal &goal)
{
	this->goal_ = goal; 
};

// Set command prototypes to use.
void SA_WorkingPlan::set_commands (AddOpenThreatsCmd *add_threats_cmd)
{
	if (add_threats_cmd == 0)
		return;

	this->add_threats_cmd_ = add_threats_cmd;
	this->has_cmds_ = true;
};

// Helper function to create next instance id.
TaskInstID SA_WorkingPlan::get_next_inst_id (void)
{
	TaskInstID inst_id = this->next_inst_id_;
	this->next_inst_id_++;
	return inst_id;
};

// Get current plan.
const Plan& SA_WorkingPlan::get_plan (void)
{
	//****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP
	// Should track whether plan has changed rather than regenerating every time.
	//****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP

	// Clear plan.
	this->plan_.clear ();

  // Set goal.
  this->plan_.goal = this->goal_;

	// Set time windows.
	this->plan_.start_window = this->goal_.start_window;

	//****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP
	// (Set absolute end window from start window + end windows of goal conditions)
  this->plan_.end_window.first = NULL_TIME;
  this->plan_.end_window.second = NULL_TIME;
	//****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP

	// Set plan ID and name from goal.
	this->plan_.plan_id = this->goal_.goal_id;
	this->plan_.name = this->goal_.name;

	// Add task instances to plan.
	for (InstToTaskMap::iterator iter =
		this->task_insts_.begin ();
		iter != this->task_insts_.end (); iter++)
	{
    // If the task instance is a placeholder, don't add it.
    if (iter->first < 0 || iter->first == GOAL_TASK_INST_ID || iter->first == INIT_TASK_INST_ID || iter->first == NULL_TASK_INST_ID)
      continue;

    // Creat task instance and set instance and task IDs from task instance map.
	  PlanTaskInst inst;
	  inst.inst_id = iter->first;
	  inst.task_id = iter->second;

	  //****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP
	  // (Need to add separate Type to task nodes instead of using name.)
	  // Set instance type.
	  inst.type_id = this->planner_->get_task_name (inst.task_id);
	  // Give instance a name.
	  inst.name = "TaskInst";
	  inst.name += to_string (inst.inst_id);
	  inst.name += "_";
	  inst.name += inst.type_id;
	  //****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP****TEMP

	  // Set start and end windows for this instance.
	  SA_WorkingPlan::InstToWinMap::iterator win_iter;
	  win_iter = this->start_window_map_.find (inst.inst_id);
	  if (win_iter != this->start_window_map_.end ())
		  inst.start_window = win_iter->second;
	  else
		  throw "SA_POP::SA_WorkingPlan::get_plan (): Unable to find start window for task instance.";
	  win_iter = this->end_window_map_.find (inst.inst_id);
	  if (win_iter != this->end_window_map_.end ())
		  inst.end_window = win_iter->second;
	  else
		  throw "SA_POP::SA_WorkingPlan::get_plan (): Unable to find end window for task instance.";

	  // Set suggested implementation for this instance.
	  InstToImplMap::iterator impl_iter;
	  impl_iter = this->task_impls_.find (inst.inst_id);
	  if (impl_iter != this->task_impls_.end ())
		  inst.suggested_impl = impl_iter->second;
	  else
		  throw "SA_POP::SA_WorkingPlan::get_plan (): Unable to find implementation from task instance.";

	  // Add instance to plan.
	  this->plan_.task_insts.insert (inst);
	}

	// Add scheduling links.
	for (SA_WorkingPlan::SchedulingLinks::iterator sched_iter =
		this->sched_links_.begin ();
		sched_iter != this->sched_links_.end (); sched_iter++)
	{
		this->plan_.sched_links.insert (std::make_pair (
			sched_iter->first, sched_iter->second));
	}

	// Add plan data connections and causal links.
	for (SA_WorkingPlan::CondToCLinksMap::iterator cl_iter = 
		this->causal_links_.begin ();
		cl_iter != this->causal_links_.end (); cl_iter++)
	{
    // If either task instance in this causal link is a placeholder, don't add connection.
    if (cl_iter->second.first < 0 || cl_iter->second.second < 0 || cl_iter->second.first == SA_POP::INIT_TASK_INST_ID || cl_iter->second.second == SA_POP::GOAL_TASK_INST_ID)
			continue;

		// If this condition is a data condition, add data connection to plan.
		if (cl_iter->first.kind == SA_POP::DATA) {
			PlanConnection plan_conn;

			// Set condition.
			plan_conn.cond = cl_iter->first.id;

			// Set task instances.
			plan_conn.first_task_inst = cl_iter->second.first;
			plan_conn.second_task_inst = cl_iter->second.second;

			// Get task ids from instances.
			InstToTaskMap::iterator inst_task_iter;

			inst_task_iter = this->task_insts_.find (cl_iter->second.first);
			if (inst_task_iter == this->task_insts_.end ())
				throw "SA_POP::SA_WorkingPlan::get_plan (): couldn't find task id for task instance to get port for plan connection.";
			TaskID first_task = inst_task_iter->second;

			inst_task_iter = this->task_insts_.find (cl_iter->second.second);
			if (inst_task_iter == this->task_insts_.end ())
				throw "SA_POP::SA_WorkingPlan::get_plan (): couldn't find task id for task instance to get port for plan connection.";
			TaskID second_task = inst_task_iter->second;

			// Get ports.
			LinkPorts ports = this->planner_->get_clink_ports
				(first_task, cl_iter->first.id, second_task);
			plan_conn.first_port = ports.first;
			plan_conn.second_port = ports.second;

			this->plan_.connections.insert (plan_conn);
		} else {  // This is a non-data condition so add causal link to plan.
			this->plan_.causal_links.insert (cl_iter->second);
		}
	}

	return this->plan_;
};

// Get set of task instances for a precedence relation to a task instance.
const TaskInstSet *SA_WorkingPlan::get_prec_set (TaskInstID task_inst, PrecedenceRelation prec_rel)
{
	PrecedenceGraph::iterator piter = this->precedence_graph_.find(prec_rel);
	PrecedenceSet::iterator titer = piter->second.find(task_inst);
	return &(titer->second);
};

// Get task id of a task instance.
TaskID SA_WorkingPlan::get_task_from_inst (TaskInstID inst_id)
{
	if(inst_id == INIT_TASK_INST_ID){
		return INIT_TASK_ID;
	}

	InstToTaskMap::iterator iter =
		this->task_insts_.find (inst_id);
	if (iter == this->task_insts_.end ())
		throw "SA_POP::SA_WorkingPlan::get_task_from_inst (): requested instance id does not exist.";

	return iter->second;
};

/// Get task implementation id of a task instance.
TaskImplID SA_WorkingPlan::get_task_impl_from_inst(TaskInstID inst_id)
{
	if(!inst_has_impl(inst_id)) return NULL_TASK_IMPL_ID;
	return this->task_impls_.find(inst_id)->second;
}

bool SA_WorkingPlan::inst_has_impl(TaskInstID inst_id){

	return !(task_impls_.find(inst_id) == task_impls_.end());
}

//Go through and find all conflicting tasks
void SA_WorkingPlan::generate_all_threats(void)
{
	threat_set.clear();

	std::ostringstream debug_text;
	debug_text << "SA_WorkingPlan::generate_all_threats:  All Causal Links: " << std::endl;



	for(CondToCLinksMap::iterator nit = causal_links_.begin(); nit != causal_links_.end(); nit++){

		CausalLink causal_threatened = (*nit).second;
		TaskID threatened_task = this->task_insts_.find(causal_threatened.first)->second;
		if(causal_threatened.second != -1)
			debug_text << "  Task ("<<threatened_task<<") " << "Inst ("<<causal_threatened.first<<") -(" << causal_threatened.cond.id << ")-> Task ("<<this->task_insts_.find(causal_threatened.second)->second<< ") Inst ("<< causal_threatened.second << ")" << std::endl;           

	}
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");


	debug_text << "SA_WorkingPlan::generate_all_threats:  All Tasks Instances: " << std::endl;

	for(InstToTaskMap::iterator iterator = this->task_insts_.begin(); iterator != this->task_insts_.end(); iterator++){
		TaskInstID threat_possibility = iterator->first;
		TaskID threat_possibility_taskid = iterator->second;

		if(iterator->first >= 858){
			bool here = true;
		}

		debug_text <<"  Task (" <<threat_possibility_taskid << ")"<< ": Inst (" <<iterator->first << ")" << std::endl;
	}
	debug_text<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	debug_text << "SA_WorkingPlan::generate_all_threats:  All Causal Threats: " << std::endl;

	//Iterate through task instances
	for(InstToTaskMap::iterator iterator = this->task_insts_.begin(); iterator != this->task_insts_.end(); iterator++){
		TaskInstID threat_possibility = iterator->first;
		TaskID threat_possibility_taskid = iterator->second;

		if(threat_possibility_taskid == INIT_TASK_ID){
			continue;
		}

		CondSet set = this->planner_->get_effects(threat_possibility_taskid);

		//Iterate through the effects of each task instance
		for(CondSet::iterator arr = set.begin(); arr != set.end(); arr++){ 

			Condition condition = (*arr);

			std::pair<
				CondToCLinksMap::iterator,
				CondToCLinksMap::iterator
			> ret = this->causal_links_.equal_range(condition);

			//Iterates through causal links
			for(CondToCLinksMap::iterator nit = ret.first; nit != ret.second; nit++){

				CausalLink causal_threatened = (*nit).second;
				TaskID threatened_task = this->task_insts_.find(causal_threatened.first)->second;

				SANet::LinkWeight threat_effect = this->planner_->get_effect_prob(threat_possibility_taskid, condition.id);

				SANet::LinkWeight causal_effect;
				if(threatened_task == INIT_TASK_ID){
					causal_effect = 2*(this->planner_->get_cond_val(causal_threatened.cond.id)-.5);
				}else{
					causal_effect = this->planner_->get_effect_prob(threatened_task, causal_threatened.cond.id);
				}

				if((threat_effect > 0 && causal_effect < 0 )|| (threat_effect < 0 && causal_effect > 0)){

					if(causal_threatened.first != threat_possibility && causal_threatened.second != threat_possibility)
					{
						bool a = false;
						bool b = false; 
						PrecedenceSet::iterator after_thr_it = (&this->precedence_graph_.find(AFTER)->second)->find(threat_possibility);
						PrecedenceSet::iterator before_thr_it= (&this->precedence_graph_.find(BEFORE)->second)->find(threat_possibility);

						TaskInstSet after_threat = after_thr_it->second;
						TaskInstSet before_threat = before_thr_it->second;

						a = after_threat.find(causal_threatened.first) != after_threat.end();
						b = before_threat.find(causal_threatened.second) != before_threat.end();

						if(!(a || b))
						{
							CLThreat new_threat;
							new_threat.clink = causal_threatened;
							new_threat.threat = threat_possibility;



							std::pair<CLThreatSet::iterator, bool> asdf = threat_set.insert(new_threat);



							TaskID threatened_task1 = this->task_insts_.find(causal_threatened.first)->second;
							if(causal_threatened.second != SA_POP::GOAL_TASK_INST_ID)
							{
								TaskID threatened_task2 = this->task_insts_.find(causal_threatened.second)->second;
								debug_text <<"  Causal link from Task ("<< threatened_task1 <<") Inst ("<< causal_threatened.first <<") to Task ("<< threatened_task2 <<") Inst ("<< causal_threatened.second <<") using condition "<<causal_threatened.cond.id<<" threatened by Task ("<< threat_possibility_taskid <<") Inst ("<<threat_possibility<<")" << std::endl;

							}
							else
							{
								debug_text <<"  Causal link from Task ("<< threatened_task1 <<") Inst ("<< causal_threatened.first <<") to Task ( GOAL_TASK )Inst (GOAL_TASK_INST) using condition "<<causal_threatened.cond.id<<" threatened by Task ("<< threat_possibility_taskid <<") Inst ("<<threat_possibility<<")" << std::endl;
							}

						}
					}
				}
			}
		}
	}


	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	print_precedence_graph("SA_WorkingPlan::generate_all_threats");

}

void SA_WorkingPlan::print_precedence_graph(std::string source){

	std::ostringstream debug_text;

	PrecedenceSet* new_befores = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet* new_afters = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet* new_simuls = &this->precedence_graph_.find(SIMUL)->second;
	PrecedenceSet* new_unrankeds = &this->precedence_graph_.find(UNRANKED)->second;

	debug_text << source<<": Precendence Sets: BEFORE: " << std::endl;
	for(PrecedenceSet::iterator it = new_befores->begin(); it != new_befores->end(); it++){
		debug_text<<"  TaskInst ID "<<it->first<<" : ";
		for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
			debug_text<<" "<<*jt;
		}
		debug_text<<std::endl;
	}
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	debug_text << source<<": Precendence Sets: AFTER: " << std::endl;
	for(PrecedenceSet::iterator it = new_afters->begin(); it != new_afters->end(); it++){
		debug_text<<"  TaskInst ID "<<it->first<<" : ";
		for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
			debug_text<<" "<<*jt;
		}
		debug_text<<std::endl;
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");


	debug_text << source<<": Precendence Sets: SIMUL: " << std::endl;
	for(PrecedenceSet::iterator it = new_simuls->begin(); it != new_simuls->end(); it++){
		debug_text<<"  TaskInst ID "<<it->first<<" : ";
		for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
			debug_text<<" "<<*jt;
		}
		debug_text<<std::endl;
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	debug_text << source<<": Precendence Sets: UNRANKED: " << std::endl;
	for(PrecedenceSet::iterator it = new_unrankeds->begin(); it != new_unrankeds->end(); it++){
		debug_text<<"  TaskInst ID "<<it->first<<" : ";
		for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
			debug_text<<" "<<*jt;
		}
		debug_text<<std::endl;
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

}


// Get all current causal link threats.
CLThreatSet SA_WorkingPlan::get_all_threats (void)
{ 
	return threat_set;
};

//Get all the causal and data links TO the task instance
CLSet SA_WorkingPlan::get_before(TaskInstID task_inst)
{
	CLSet ret;
	for(CondToCLinksMap::iterator iter = this->causal_links_.begin();iter!=this->causal_links_.end();iter++)
		if(iter->second.second == task_inst) ret.insert(iter->second);
	return ret;
}
//Get all the causal and data links FROM the task instance
CLSet SA_WorkingPlan::get_after(TaskInstID task_inst)
{
	CLSet ret;
	for(CondToCLinksMap::iterator iter = this->causal_links_.begin();iter!=this->causal_links_.end();iter++)
		if(iter->second.second != GOAL_TASK_INST_ID && iter->second.second != NULL_TASK_INST_ID && iter->second.first == task_inst) ret.insert(iter->second);
	return ret;
}

/// Get the Causal and Scheduling orderings to this task instance
TaskInstSet SA_WorkingPlan::before_orderings (TaskInstID inst_id)
{
	CLSet before_links = this->get_before(inst_id);
	TaskInstSet ret;
	for(CLSet::iterator iter=before_links.begin();iter!=before_links.end();iter++)
	{
		if(iter->cond.kind!=DATA) ret.insert(iter->first);
	}
	for(SchedulingLinks::iterator iter=sched_links_.begin();iter!=sched_links_.end();iter++)
		if(iter->second==inst_id) ret.insert(iter->first);
	return ret;
}
/// Get the Causal and Scheduling orderings from this task instance
TaskInstSet SA_WorkingPlan::after_orderings (TaskInstID inst_id)
{
	CLSet after_links = this->get_after(inst_id);
	TaskInstSet ret;
	for(CLSet::iterator iter=after_links.begin();iter!=after_links.end();iter++)
		if(iter->cond.kind!=DATA) ret.insert(iter->second);
	for(SchedulingLinks::iterator iter=sched_links_.begin();iter!=sched_links_.end();iter++)
		if(iter->first==inst_id) ret.insert(iter->second);
	return ret;
}
/// Get a PlanCommand prototype for adding a task to the plan.
AddTaskCmd *SA_WorkingPlan::get_AddTaskCmd (void)
{
	return static_cast<AddTaskCmd *> (this->add_task_cmd_->clone ());
};

// Get a PlanCommand prototype for associating an implementation with a
// task instance in the plan.
AssocTaskImplCmd *SA_WorkingPlan::get_AssocTaskImplCmd (void)
{
	return static_cast<AssocTaskImplCmd *> (this->assoc_impl_cmd_->clone ());
};

// Get a PlanCommand prototype for resolving a causal link threat in the
// plan (with promotion or demotion).
ResolveCLThreatCmd *SA_WorkingPlan::get_ResolveCLThreatCmd (void)
{
	return static_cast<ResolveCLThreatCmd *> (this->resolve_threat_cmd_->clone ());
};

// Get a PlanCommand prototype for resolving a scheduling conflict (i.e.
// non-causal-link ordering constraint (with promotion or demotion)
// between two task instances in the plan.
ResolveSchedOrderCmd *SA_WorkingPlan::get_ResolveSchedOrderCmd (void)
{
	return static_cast<ResolveSchedOrderCmd *> (this->resolve_sched_cmd_->clone ());
};

// Get a PlanCommand prototype for adjusting the minimum times of a
// task instance in the plan.
AdjustMinTimesCmd *SA_WorkingPlan::get_AdjustMinTimesCmd (void)
{
	return static_cast<AdjustMinTimesCmd *> (this->adj_min_times_cmd_->clone ());
};

// Get a PlanCommand prototype for adjusting the maximum times of a
// task instance in the plan.
AdjustMaxTimesCmd *SA_WorkingPlan::get_AdjustMaxTimesCmd (void)
{
	return static_cast<AdjustMaxTimesCmd *> (this->adj_max_times_cmd_->clone ());
};

void SA_WorkingPlan::execute (SA_AddTaskCmd *cmd)
{

	TaskChoice task_choice = cmd->tasks_.front();
	TaskID task = task_choice.task_id;
	TaskInstID task_inst;
	Condition cond = cmd->cond_;

	cmd->last_task_choice_ = task_choice;



	if(task_choice.choice == REUSE_INST){

		task_inst = task_choice.task_inst_id;
		this->reused_insts_.insert(task_inst);
		cmd->tasks_.pop_front();
	}
	else if(task_choice.choice == NEW_INST){

	//	if(task == INIT_TASK_ID && this->planner_->init_added)
	//	{
	//		throw "Reached SA_WorkingPlan::execute (SA_AddTaskCmd *cmd) for Special Initial Action after it was already existing instance tried";
	//	}

		if(task == INIT_TASK_ID){
	//		this->planner_->init_added =  true;
			bool asdf_problem = true;
		}

		task_inst = this->get_next_inst_id ();
		// Add task instance.
		this->task_insts_.insert (std::make_pair (task_inst, task));
		//   Remove this task from tasks still to try.
		cmd->tasks_.pop_front ();
	}



	std::ostringstream debug_text;
	debug_text << "SA_WorkingPlan::execute (SA_AddTaskCmd *cmd): Adding task (" << task << ") instance (" << task_inst << ") for condition (" << cond.id << ").";

	SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());

	// Create causal links for each task instance depending on the condition.
	for (TaskInstSet::iterator inst_iter = cmd->task_insts_.begin ();
		inst_iter != cmd->task_insts_.end (); inst_iter++)
	{
		CausalLink clink;
		clink.cond = cond;
		clink.first = task_inst;
		clink.second = *inst_iter;
		CondToCLinksMap::iterator links_iter;

		// Check whether this causal link already exists in working plan.
		for (links_iter = this->causal_links_.lower_bound (cond);links_iter != this->causal_links_.upper_bound (cond);links_iter++)
			if(links_iter->second == clink)
				break;
		// If causal link not found in working plan, add it to causal links
		if(links_iter==this->causal_links_.upper_bound(cond))
		{
			std::ostringstream debug_text2;
			debug_text2 << "SA_WorkingPlan::execute (SA_AddTaskCmd *cmd): Adding causal link (" << clink.first << " -" << clink.cond.id << "-> " << clink.second << ")";
			SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text2.str ());

			this->causal_links_.insert (std::make_pair (cond, clink));
			this->causal_links_by_first.insert (std::make_pair (clink.first, clink));
			this->causal_links_by_second.insert (std::make_pair (clink.second, clink));

			cmd->added_links_.insert(clink);
		}
	}

	// Set this task and task instance as last used by command.
	cmd->last_task_ = task;
	cmd->last_task_inst_ = task_inst;
};

//Check to see if a condition is suspended
bool SA_WorkingPlan::condition_in_suspended(Condition condition, TaskInstID required_by){

	std::pair<Condition, TaskInstID> condition_pair(condition, required_by);

	return (this->suspended_conditions.find(condition_pair) != this->suspended_conditions.end());
}

//Helps with checking if a condition needs to be suspended
CausalLink SA_WorkingPlan::clink_on_path(Condition condition, TaskInstID required_by){
	std::set<TaskInstID> expanded;
	expanded.clear();

	return clink_on_path_aux(condition, required_by, expanded);
}

CausalLink SA_WorkingPlan::clink_on_path_aux(Condition condition, TaskInstID required_by, 
											 std::set<TaskInstID> & expanded){

												 expanded.insert(required_by);

												 if(required_by == -1){
													 return this->no_link;
												 }

												 for(TaskToCLinksMap::iterator it = causal_links_by_first.lower_bound(required_by);
													 it != causal_links_by_first.upper_bound(required_by); it++){

														 if(expanded.find(it->second.second) != expanded.end()){
															 continue;
														 }

														 if(it->second.cond == condition){
															 return it->second;
														 }

														 CausalLink to_return = clink_on_path_aux(condition, it->second.second, expanded);

														 if(to_return != no_link){
															 return to_return;
														 }
												 }

												 return no_link;
}

//When a task is added, unsuspend tasks on that path
void SA_WorkingPlan::unsuspend_listeners(CausalLink link, TaskInstID exception){

	for(TaskToCLinksMap::iterator it = this->causal_links_by_first.lower_bound(link.second); 
		it != this->causal_links_by_first.upper_bound(link.second); it++){

			unsuspend_listeners_aux(it->second, exception);
	}
}

//Auxillery for unsuspending
void SA_WorkingPlan::unsuspend_listeners_aux(CausalLink link, TaskInstID exception){

	for(SuspendedConditionListenerMap::iterator it = this->suspended_listener_map.lower_bound(link);
		it != this->suspended_listener_map.upper_bound(link);){

			SuspendedConditionListenerMap::iterator prev_it = it++;

			if(exception != prev_it->second.second){
				this->suspended_conditions.erase(prev_it->second);
				suspended_listener_map.erase(prev_it);
			}
	}



	for(TaskToCLinksMap::iterator it = this->causal_links_by_first.lower_bound(link.second); 
		it != this->causal_links_by_first.upper_bound(link.second); it++){
			unsuspend_listeners_aux(it->second, exception);
	}
}

//Suspend a condition
void SA_WorkingPlan::suspend_condition(Condition cond, TaskInstID required_by, CausalLink suspended_by){

	std::pair<Condition, TaskInstID> require_set (cond, required_by);
	std::pair<CausalLink, std::pair<Condition, TaskInstID> > to_insert (suspended_by, require_set);

	this->suspended_listener_map.insert(to_insert);
	this->suspended_conditions.insert(require_set);
}

//Note that a condition should be removed from the suspend map
void SA_WorkingPlan::resume_condition(Condition cond, TaskInstID required_by, CausalLink suspended_by){
	std::pair<Condition, TaskInstID> require_set (cond, required_by);
	std::pair<CausalLink, std::pair<Condition, TaskInstID> > to_insert (suspended_by, require_set);

	std::multimap<CausalLink, std::pair<Condition, TaskInstID> > ::iterator it;

	for(it = suspended_listener_map.lower_bound(suspended_by);
		it != suspended_listener_map.upper_bound(suspended_by); it++){
			if(it->second == require_set){
				break;
			}
	}

	suspended_listener_map.erase(it);

	this->suspended_conditions.erase(require_set);
}

void SA_WorkingPlan::undo (SA_AddTaskCmd *cmd)
{


	if(cmd->last_task_choice_.choice == NEW_INST){
		this->task_insts_.erase (this->task_insts_.find(cmd->last_task_inst_));  
	}else{
		this->reused_insts_.erase(this->reused_insts_.find(cmd->last_task_inst_));
	}


	for (SA_WorkingPlan::CondToCLinksMap::iterator cl_iter = this->causal_links_.lower_bound (cmd->cond_);
		cl_iter != this->causal_links_.upper_bound (cmd->cond_);)
	{
		SA_WorkingPlan::CondToCLinksMap::iterator prev_iter = cl_iter;
		cl_iter++;
		if (cmd->task_insts_.find (prev_iter->second.second) != cmd->task_insts_.end ())
		{
			for(CLSet::iterator iter = cmd->added_links_.begin();iter!=cmd->added_links_.end();iter++)
			{
				if(prev_iter->second == *iter)  
				{

					CausalLink clink = prev_iter->second;

					this->causal_links_.erase (prev_iter);

					break;
				}
			}
		}
	}

	for (SA_WorkingPlan::TaskToCLinksMap::iterator cl_iter = this->causal_links_by_first.lower_bound (cmd->last_task_choice_.task_inst_id);
		cl_iter != this->causal_links_by_first.upper_bound (cmd->last_task_choice_.task_inst_id);)
	{
		SA_WorkingPlan::TaskToCLinksMap::iterator prev_iter = cl_iter;
		cl_iter++;
		if (cmd->task_insts_.find (prev_iter->second.second) != cmd->task_insts_.end ())
		{
			for(CLSet::iterator iter = cmd->added_links_.begin();iter!=cmd->added_links_.end();iter++)
			{
				if(prev_iter->second == *iter)  
				{

					CausalLink clink = prev_iter->second;

					this->causal_links_by_first.erase (prev_iter);

					break;
				}
			}
		}
	}

	TaskInstID to_find = *(cmd->get_satisfied_tasks().begin());

	for (SA_WorkingPlan::TaskToCLinksMap::iterator cl_iter = this->causal_links_by_second.lower_bound (to_find);
		cl_iter != this->causal_links_by_second.upper_bound (to_find);)
	{
		SA_WorkingPlan::TaskToCLinksMap::iterator prev_iter = cl_iter;
		cl_iter++;
		if (cmd->task_insts_.find (prev_iter->second.second) != cmd->task_insts_.end ())
		{
			for(CLSet::iterator iter = cmd->added_links_.begin();iter!=cmd->added_links_.end();iter++)
			{
				if(prev_iter->second == *iter)  
				{

					CausalLink clink = prev_iter->second;

					this->causal_links_by_second.erase (prev_iter);
					cmd->added_links_.erase(iter);
					break;
				}
			}
		}
	}
};

bool SA_WorkingPlan::is_null_link(CausalLink link){
	return link == no_link;
}

bool SA_WorkingPlan::is_null_condition(Condition cond){
	return cond == no_condition;
}

Condition SA_WorkingPlan::get_no_condition(){
	return no_condition;
}

// Execute a command to associate an implementation with a
// task instance in the plan.
bool SA_WorkingPlan::execute (SA_AssocTaskImplCmd *cmd)
{


	// Associate task instance with first implementation in list from command.
	if(this->reused_insts_.find(cmd->task_inst_)==this->reused_insts_.end())
	{
		this->task_impls_.insert (std::make_pair(cmd->task_inst_, cmd->impls_.front ()));
		this->durations.insert(std::make_pair(cmd->task_inst_, this->planner_->get_impl(cmd->impls_.front())->get_duration()));


	}

	if(cmd->task_inst_ == INIT_TASK_INST_ID){
		this->durations.insert(std::make_pair(cmd->task_inst_, 1));
	}

	// Update last implementation to this one and remove it from command.
	cmd->last_impl_ = cmd->impls_.front ();
	cmd->impls_.pop_front ();
	// Update the precedence graph

	PrecedenceSet* befores = &this->precedence_graph_.find(BEFORE)->second;

	PrecedenceSet::iterator find_inst = befores->find(cmd->task_inst_);
	if(find_inst != befores->end()){
		TaskInstSet before_task = find_inst->second;

		for(TaskInstSet::iterator it = cmd->satisfied_insts.begin(); it != cmd->satisfied_insts.end(); it++){
			if(before_task.find(*it) != before_task.end()){
				return false;
			}
		}
	}

	cmd->got_to_scheduling = true;


	cmd->saved_listener_map = suspended_listener_map;
	cmd->saved_suspended_set = suspended_conditions;

	for(CLSet::iterator it = cmd->added_links.begin(); it != cmd->added_links.end(); it++){
		unsuspend_listeners(*it, it->first);
	}

	bool toReturn	= this->init_prec_insert(cmd->task_inst_,cmd);

	return toReturn;

};

// Undo a command to associate an implementation with a
// task instance in the plan.
void SA_WorkingPlan::undo (SA_AssocTaskImplCmd *cmd)
{
	// Undo the time window adjustments and the precedence graph updations.
	PrecedenceSet* befores = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet* afters = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet* simuls = &this->precedence_graph_.find(SIMUL)->second;
	PrecedenceSet* unrankeds = &this->precedence_graph_.find(UNRANKED)->second;


	if(cmd->got_to_scheduling){
		suspended_listener_map = cmd->saved_listener_map;
		suspended_conditions = cmd->saved_suspended_set;

		this->undo(&cmd->max_adj_cmd);
		this->undo(&cmd->min_adj_cmd);
		this->prec_erase(cmd->task_inst_,cmd);
		cmd->causal_insertions.clear();
		cmd->simul_insertions.clear();


	}

	if(this->reused_insts_.find(cmd->task_inst_)==this->reused_insts_.end())
	{
		this->durations.erase(this->durations.find(cmd->task_inst_));
		this->task_impls_.erase (cmd->task_inst_);
	}
};

PrecedenceGraph SA_WorkingPlan::get_precedence_graph(){
	return this->precedence_graph_;
}

// Execute a command to resolve a causal link threat in the
// plan (with promotion or demotion).
bool SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd)
{


	std::ostringstream debug_text;


	TaskInstID first_task_inst = cmd->first;
	TaskInstID second_task_inst = cmd->second;
	Condition condition = cmd->condition;


	if(cmd->second == INIT_TASK_INST_ID){

		debug_text << "SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd):  Cannot schedule before initial task"<<std::endl;
		SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
		debug_text.str("");

		return false;
	}

	if(cmd->first == -1){

		debug_text << "SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd):  Cannot push threat to after goal"<<std::endl;
		SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
		debug_text.str("");

		if(cmd->second == 12){
			bool a = true;
		}

		return false;
	}

	PrecedenceSet* befores = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet* afters = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet* simuls = &this->precedence_graph_.find(SIMUL)->second;
	PrecedenceSet* unrankeds = &this->precedence_graph_.find(UNRANKED)->second;


	TaskInstSet *before_A = &befores->find(cmd->first)->second;
	TaskInstSet *after_A = &afters->find(cmd->first)->second;
	TaskInstSet *simul_A = &simuls->find(cmd->first)->second;
	TaskInstSet *unranked_A = &unrankeds->find(cmd->first)->second;

	TaskInstSet *before_B = &befores->find(cmd->second)->second;
	TaskInstSet *after_B = &afters->find(cmd->second)->second;
	TaskInstSet *simul_B = &simuls->find(cmd->second)->second;
	TaskInstSet *unranked_B = &unrankeds->find(cmd->second)->second;


	if(before_B->find(cmd->first) != before_B->end()){
		//Don't need to resolve

		debug_text << "SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd):  Threat has already been resolved: "<<cmd->first<<" is before "<<cmd->second<<std::endl;
		SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
		debug_text.str("");

		return true;
	}

	if(after_B->find(cmd->first) != after_B->end()){
		//impossible?
		debug_text << "SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd):  Cannot schedule task inst"<<cmd->first<<" before task inst"<<cmd->second<<std::endl;
		SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
		debug_text.str("");

		return false;

	}

	cmd->got_to_change_precedences = true;

	cmd->befores = *befores;
	cmd->afters = *afters;
	cmd->simuls = *simuls;
	cmd->unrankeds = *unrankeds;


	TaskInstSet tmp;

	before_B->insert(cmd->first);
	unranked_B->erase(cmd->first);

	for(TaskInstSet::iterator it = before_A->begin(); it != before_A->end(); it++){

		before_B->insert(*it);
		unranked_B->erase(*it);

		afters->find(*it)->second.insert(cmd->second);
		unrankeds->find(*it)->second.erase(cmd->second);


		for(TaskInstSet::iterator it2 = after_B->begin(); it2 != after_B->end(); it2++){
			befores->find(*it2)->second.insert(*it);
			unrankeds->find(*it2)->second.erase(*it);

			afters->find(*it)->second.insert(*it2);
			unrankeds->find(*it)->second.erase(*it2);
		}
	}

	after_A->insert(cmd->second);
	unranked_A->erase(cmd->second);

	for(TaskInstSet::iterator it = after_B->begin(); it != after_B->end(); it++){

		after_A->insert(*it);
		unranked_A->erase(*it);

		befores->find(*it)->second.insert(cmd->first);
		unrankeds->find(*it)->second.erase(cmd->first);

	}


	debug_text << "SA_WorkingPlan::execute (SA_ResolveCLThreatCmd * cmd): Now scheduling task "<<cmd->first<<" before "<<cmd->second<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
	debug_text.str("");

	return true;
};

// Undo a command to resolve a causal link threat in the
// plan (with promotion or demotion).
void SA_WorkingPlan::undo (SA_ResolveCLThreatCmd * cmd)
{

	std::ostringstream debug_text;

	debug_text << "SA_WorkingPlan::undo (SA_ResolveCLThreatCmd * cmd): Undoing scheduling task "<<cmd->first<<" before "<<cmd->second<<std::endl;
	if(!cmd->got_to_change_precedences){
		debug_text <<"		no changes to the precedence graph to undo"<<std::endl;

		SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
		debug_text.str("");
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
	debug_text.str("");

	if(cmd->got_to_change_precedences){


		PrecedenceSet* new_befores = &this->precedence_graph_.find(BEFORE)->second;
		PrecedenceSet* new_afters = &this->precedence_graph_.find(AFTER)->second;
		PrecedenceSet* new_simuls = &this->precedence_graph_.find(SIMUL)->second;
		PrecedenceSet* new_unrankeds = &this->precedence_graph_.find(UNRANKED)->second;

		for(PrecedenceSet::iterator it = cmd->befores.begin(); it != cmd->befores.end(); it++){
			new_befores->find(it->first)->second.clear();
			for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
				new_befores->find(it->first)->second.insert(*jt);
			}
		}

		for(PrecedenceSet::iterator it = cmd->afters.begin(); it != cmd->afters.end(); it++){
			new_afters->find(it->first)->second.clear();
			for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
				new_afters->find(it->first)->second.insert(*jt);
			}
		}

		for(PrecedenceSet::iterator it = cmd->simuls.begin(); it != cmd->simuls.end(); it++){
			new_simuls->find(it->first)->second.clear();
			for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
				new_simuls->find(it->first)->second.insert(*jt);
			}
		}

		for(PrecedenceSet::iterator it = cmd->unrankeds.begin(); it != cmd->unrankeds.end(); it++){
			new_unrankeds->find(it->first)->second.clear();
			for(TaskInstSet::iterator jt = it->second.begin(); jt != it->second.end(); jt++){
				new_unrankeds->find(it->first)->second.insert(*jt);
			}
		}

	}
};

// Execute a command to resolve a scheduling conflict (i.e.
// non-causal-link ordering constraint with promotion or demotion)
// between two task instances in the plan.
bool SA_WorkingPlan::execute (SA_ResolveSchedOrderCmd *cmd)
{
	std::ostringstream debug_text;

	debug_text<<"Scheduling "<<cmd->first<<" before "<<cmd->second<<std::endl;

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	TaskInstID first_task_inst = cmd->first;
	TaskInstID second_task_inst = cmd->second;
	TimeWindow first_start,second_start;
	TimeWindow first_end,second_end;
	first_start = this->get_start_window(first_task_inst);
	first_end = this->get_end_window(first_task_inst);
	second_start = this->get_start_window(second_task_inst);
	second_end = this->get_end_window(second_task_inst);

	if(second_start.second!= NULL_TIME && (first_end.second==NULL_TIME || second_start.second<first_end.second))
	{
		// There is a need to adjust the max times
		cmd->adj_max_times_cmd_ = static_cast<SA_AdjustMaxTimesCmd *> (this->get_AdjustMaxTimesCmd());
		cmd->adj_max_times_cmd_->set_times(first_task_inst,second_start.second-this->get_duration(first_task_inst),second_start.second);
		if(!this->execute(cmd->adj_max_times_cmd_))
			return false;
	}
	else cmd->adj_max_times_cmd_=NULL;
	if(first_end.first!= NULL_TIME && second_start.first<first_end.first)
	{
		// There is a need to adjust the min times
		cmd->adj_min_times_cmd_ = static_cast<SA_AdjustMinTimesCmd *> (this->get_AdjustMinTimesCmd());
		cmd->adj_min_times_cmd_->set_times(second_task_inst,first_end.first,first_end.first+this->get_duration(second_task_inst));
		if(!this->execute(cmd->adj_min_times_cmd_))
			return false;
	}
	else cmd->adj_min_times_cmd_=NULL;

	PrecedenceSet *before = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet *after = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet *simul = &this->precedence_graph_.find(SIMUL)->second;
	PrecedenceSet *unranked = &this->precedence_graph_.find(UNRANKED)->second;

	before->find(second_task_inst)->second.insert(first_task_inst);
	after->find(first_task_inst)->second.insert(second_task_inst);
	unranked->find(first_task_inst)->second.erase(unranked->find(first_task_inst)->second.find(second_task_inst));
	unranked->find(second_task_inst)->second.erase(unranked->find(second_task_inst)->second.find(first_task_inst));

	TaskInstSet after_second = this->after_orderings(second_task_inst);
	for(TaskInstSet::iterator iter=simul->find(second_task_inst)->second.begin();iter!=simul->find(second_task_inst)->second.end();iter++)
		after_second.insert(*iter);
	TaskInstSet before_first = this->before_orderings(first_task_inst);
	for(TaskInstSet::iterator iter=simul->find(first_task_inst)->second.begin();iter!=simul->find(first_task_inst)->second.end();iter++)
		before_first.insert(*iter);
	// All the task instances after and simultaneous to the second task instance should
	// be after all the task instances before and simultaneous to the first task instance
	for(TaskInstSet::iterator iter=after_second.begin();iter!=after_second.end();iter++)
	{
		if(before->find(*iter)->second.find(first_task_inst)==before->find(*iter)->second.end())
		{
			SA_ResolveSchedOrderCmd *temp = static_cast<SA_ResolveSchedOrderCmd *> (this->get_ResolveSchedOrderCmd());
			temp->set_task_insts(first_task_inst,*iter);
			cmd->cmds_.push_back(temp);
			if(!this->execute(temp)) return false;
		}
	}
	// All the task instances before and simultaneous to the first task instance should
	// be before all the task instances after and simultaneous to the second task instance
	for(TaskInstSet::iterator iter=before_first.begin();iter!=before_first.end();iter++)
	{
		if(after->find(*iter)->second.find(second_task_inst)==after->find(*iter)->second.end())
		{
			SA_ResolveSchedOrderCmd *temp = static_cast<SA_ResolveSchedOrderCmd *> (this->get_ResolveSchedOrderCmd());
			temp->set_task_insts(*iter,second_task_inst);
			cmd->cmds_.push_back(temp);
			if(!this->execute(temp)) return false;
		}
	}
	return true;
};

// Undo a command to resolve a scheduling conflict (i.e.
// non-causal-link ordering constraint with promotion or demotion)
// between two task instances in the plan.
void SA_WorkingPlan::undo (SA_ResolveSchedOrderCmd *cmd)
{
	for(std::list<SA_ResolveSchedOrderCmd*>::reverse_iterator iter=cmd->cmds_.rbegin();iter!=cmd->cmds_.rend();iter++)
		this->undo(*iter);

	TaskInstID first_task_inst = cmd->first;
	TaskInstID second_task_inst = cmd->second;

	PrecedenceSet *before = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet *after = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet *unranked = &this->precedence_graph_.find(UNRANKED)->second;

	before->find(second_task_inst)->second.erase(before->find(second_task_inst)->second.find(first_task_inst));
	after->find(first_task_inst)->second.erase(after->find(first_task_inst)->second.find(second_task_inst));
	unranked->find(first_task_inst)->second.insert(second_task_inst);
	unranked->find(second_task_inst)->second.insert(first_task_inst);
	if(cmd->adj_max_times_cmd_ != NULL) {
		this->undo(cmd->adj_max_times_cmd_);
	}
	if(cmd->adj_min_times_cmd_ != NULL)this->undo(cmd->adj_min_times_cmd_);
};

// Execute a command to adjust the minimum times of a
// task instance in the plan.
bool SA_WorkingPlan::execute (SA_AdjustMinTimesCmd *cmd)
{
	std::ostringstream debug_text;
	debug_text<<"adjusting min time for "<<cmd->task_inst_<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	TimeWindow *start_win = &this->start_window_map_.find(cmd->task_inst_)->second;
	TimeWindow *end_win = &this->end_window_map_.find(cmd->task_inst_)->second;
	// If this adjustment violated and time constraints, return false.
	if(start_win->second!=NULL_TIME &&(cmd->new_end_min==NULL_TIME || cmd->new_end_min>end_win->second || cmd->new_start_min>start_win->second)) 
		return false;
	// Set the new time window
	start_win->first = cmd->new_start_min;
	end_win->first = cmd->new_end_min;

	TimeWindow ts = this->get_start_window(cmd->task_inst_);
	TimeWindow es = this->get_end_window(cmd->task_inst_);
	CLSet after_links = this->get_after(cmd->task_inst_);
	if(after_links.empty()) debug_text<<"after links is empty"<<std::endl;
	TaskInstSet causal,sched,data;
	for(CLSet::iterator iter=after_links.begin();iter!=after_links.end();iter++)
	{    if(iter->cond.kind!=DATA) causal.insert(iter->second);
	else data.insert(iter->second);
	}
	for(SchedulingLinks::iterator iter=sched_links_.begin();iter!=sched_links_.end();iter++)
		if(iter->first==cmd->task_inst_) sched.insert(iter->second);


	// Propagate this time window change to all the task instances ordered after this task instance
	for(TaskInstSet::iterator iter=causal.begin();iter!=causal.end();iter++)
	{
		TimeWindow temp_start = this->get_start_window(*iter);
		TimeWindow temp_end = this->get_end_window(*iter);
		if(end_win->first>temp_start.first)
		{
			debug_text<<"adjusting time for causal "<<*iter<<std::endl;
			SA_AdjustMinTimesCmd* temp = static_cast<SA_AdjustMinTimesCmd *> (this->get_AdjustMinTimesCmd());
			TimeValue dur = this->durations.find(*iter)->second;
			if(dur!=0) temp->set_times(*iter,end_win->first,end_win->first+this->durations.find(*iter)->second);
			else temp->set_times(*iter,end_win->first,temp_end.first);
			cmd->min_adjust_cmds.push_back(temp);
		}
	}

	if(sched.empty()) debug_text<<"sched is empty"<<std::endl;

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	for(TaskInstSet::iterator iter=sched.begin();iter!=sched.end();iter++)
	{
		TimeWindow temp_start = this->get_start_window(*iter);
		TimeWindow temp_end = this->get_end_window(*iter);
		if(end_win->first>temp_start.first)
		{
			SA_AdjustMinTimesCmd* temp = static_cast<SA_AdjustMinTimesCmd *> (this->get_AdjustMinTimesCmd());
			TimeValue dur = this->durations.find(*iter)->second;
			if(dur!=0) 
				temp->set_times(*iter,end_win->first,end_win->first+this->durations.find(*iter)->second);
			else 
				temp->set_times(*iter,end_win->first,temp_end.first);
			cmd->min_adjust_cmds.push_back(temp);

		}
	}


	// Do the same change for the task instances simultaneous to this one
	for(TaskInstSet::iterator iter=data.begin();iter!=data.end();iter++)
	{
		TimeWindow temp_start = this->get_start_window(*iter);
		if(start_win->first>temp_start.first)
		{
			debug_text<<"adjusting time for data "<<*iter<<std::endl;
			SA_AdjustMinTimesCmd* temp = static_cast<SA_AdjustMinTimesCmd *> (this->get_AdjustMinTimesCmd());
			temp->set_times(*iter,start_win->first,end_win->first);
			cmd->min_adjust_cmds.push_back(temp);
		}
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	for(SA_AdjustMinTimesCmd::MinTimesAdjustList::iterator iter=cmd->min_adjust_cmds.begin();iter!=cmd->min_adjust_cmds.end();iter++)
	{
		if(!this->execute(*iter)) return false;
	}

	debug_text<<"the task inst is "<<cmd->task_inst_<<std::endl;
	// Check to see whether any task instance should be removed from the unranked set due to this time window change
	TaskInstSet *unranked = &this->precedence_graph_.find(UNRANKED)->second.find(cmd->task_inst_)->second;
	for(TaskInstSet::iterator iter=unranked->begin();iter!=unranked->end();iter++)
	{
		TimeWindow temp_end = this->get_end_window(*iter);
		if(temp_end.second!=NULL_TIME && temp_end.second<start_win->first)
		{
			this->precedence_graph_.find(BEFORE)->second.find(cmd->task_inst_)->second.insert(*iter);
			this->precedence_graph_.find(AFTER)->second.find(*iter)->second.insert(cmd->task_inst_);
			this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second.erase(this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second.find(*iter));
			cmd->ranked.insert(*iter);
		}
	}
	for(TaskInstSet::iterator iter=cmd->ranked.begin();iter!=cmd->ranked.end();iter++)
	{
		unranked->erase(unranked->find(*iter));
	}
	debug_text<<"min times of "<<cmd->task_inst_<<" : "<<cmd->new_start_min<<" "<<cmd->new_end_min<<std::endl;

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	return true;
};

// Undo a command to adjust the minimum times of a
// task instance in the plan.
void SA_WorkingPlan::undo (SA_AdjustMinTimesCmd *cmd)
{
	TimeWindow *start_win = &this->get_start_window(cmd->task_inst_);
	TimeWindow *end_win = &this->get_end_window(cmd->task_inst_);

	if(start_win->first==cmd->old_start_min && end_win->first==cmd->old_end_min) return;
	// Undo all the removal of the tasks from the unranked set
	TaskInstSet *unranked1 = &this->precedence_graph_.find(UNRANKED)->second.find(cmd->task_inst_)->second;
	TaskInstSet *before = &this->precedence_graph_.find(BEFORE)->second.find(cmd->task_inst_)->second;
	for(TaskInstSet::iterator iter=cmd->ranked.begin();iter!=cmd->ranked.end();iter++)
	{
		TaskInstSet *unranked2 = &this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second;
		TaskInstSet *after = &this->precedence_graph_.find(AFTER)->second.find(*iter)->second;
		unranked1->insert(*iter);
		unranked2->insert(cmd->task_inst_);
		before->erase(before->find(*iter));
		after->erase(after->find(cmd->task_inst_));
	}

	// Undo all the time window changes caused by this command
	for(SA_AdjustMinTimesCmd::MinTimesAdjustList::reverse_iterator iter=cmd->min_adjust_cmds.rbegin();iter!=cmd->min_adjust_cmds.rend();iter++)
	{
		this->undo(*iter);
	}
	// Reset the time window to the old one  
	start_win->first=cmd->old_start_min;
	end_win->first=cmd->old_end_min;

	return;
};

// Execute a command to adjust the maximum times of a
// task instance in the plan.
bool SA_WorkingPlan::execute (SA_AdjustMaxTimesCmd *cmd)
{
	std::ostringstream debug_text;

	debug_text<<"adjusting max time for "<<cmd->task_inst_<<std::endl;
	TimeWindow *start_win = &this->start_window_map_.find(cmd->task_inst_)->second;
	TimeWindow *end_win = &this->end_window_map_.find(cmd->task_inst_)->second;
	debug_text<<"for "<<cmd->task_inst_<<" "<<cmd->new_end_max<<" "<<end_win->first<<" "<<cmd->new_start_max<<" "<<start_win->first<<std::endl;
	// If this adjustment violated and time constraints, return false.
	if((end_win->first==NULL_TIME && cmd->new_end_max!= NULL_TIME) || cmd->new_end_max<end_win->first || cmd->new_start_max<start_win->first) return false;
	// Set the new time window
	start_win->second = cmd->new_start_max;
	end_win->second = cmd->new_end_max;
	CLSet before_links = this->get_before(cmd->task_inst_);
	TaskInstSet causal,sched,data;
	for(CLSet::iterator iter=before_links.begin();iter!=before_links.end();iter++)
	{    if(iter->cond.kind!=DATA) causal.insert(iter->first);
	else data.insert(iter->first);
	}
	for(SchedulingLinks::iterator iter=sched_links_.begin();iter!=sched_links_.end();iter++)
		if(iter->second==cmd->task_inst_) sched.insert(iter->first);

	// Propagate this time window change to all the task instances ordered before this task instance
	for(TaskInstSet::iterator iter=causal.begin();iter!=causal.end();iter++)
	{
		TimeWindow temp_end = this->get_end_window(*iter);
		if(temp_end.second==NULL_TIME || (temp_end.second!=NULL_TIME && start_win->second<temp_end.second))
		{
			SA_AdjustMaxTimesCmd* temp = static_cast<SA_AdjustMaxTimesCmd *> (this->get_AdjustMaxTimesCmd());
			temp->set_times(*iter,start_win->second-this->durations.find(*iter)->second,start_win->second);
			cmd->max_adjust_cmds.push_back(temp);
		}
	}

	if(sched.empty()) debug_text<<"msched is empty ("<<cmd->task_inst_<<")"<<std::endl;
	else
		debug_text<<"mnot sched is empty ("<<cmd->task_inst_<<")"<<std::endl;

	for(TaskInstSet::iterator iter=sched.begin();iter!=sched.end();iter++)
	{
		TimeWindow temp_end = this->get_end_window(*iter);
		if(temp_end.second==NULL_TIME || (temp_end.second!=NULL_TIME && start_win->second<temp_end.second))
		{
			SA_AdjustMaxTimesCmd* temp = static_cast<SA_AdjustMaxTimesCmd *> (this->get_AdjustMaxTimesCmd());
			temp->set_times(*iter,start_win->second-this->durations.find(*iter)->second,start_win->second);
			cmd->max_adjust_cmds.push_back(temp);
		}
	}



	// Do the same change for the task instances simultaneous to this one
	for(TaskInstSet::iterator iter=data.begin();iter!=data.end();iter++)
	{
		TimeWindow temp_end = this->get_end_window(*iter);
		if(temp_end.second==NULL_TIME || (temp_end.second!=NULL_TIME && end_win->second<temp_end.second))
		{
			SA_AdjustMaxTimesCmd* temp = static_cast<SA_AdjustMaxTimesCmd *> (this->get_AdjustMaxTimesCmd());
			temp->set_times(*iter,start_win->second,end_win->second);
			cmd->max_adjust_cmds.push_back(temp);
		}
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");


	for(SA_AdjustMaxTimesCmd::MaxTimesAdjustList::iterator iter=cmd->max_adjust_cmds.begin();iter!=cmd->max_adjust_cmds.end();iter++)
	{
		if(!this->execute(*iter)) return false;
	}
	// Check to see whether any task instance should be removed from the unranked set due to this time window change
	TaskInstSet *unranked = &this->precedence_graph_.find(UNRANKED)->second.find(cmd->task_inst_)->second;
	for(TaskInstSet::iterator iter=unranked->begin();iter!=unranked->end();iter++)
	{
		TimeWindow temp_start = this->get_start_window(*iter);
		if(temp_start.first>end_win->second)
		{
			this->precedence_graph_.find(AFTER)->second.find(cmd->task_inst_)->second.insert(*iter);
			this->precedence_graph_.find(BEFORE)->second.find(*iter)->second.insert(cmd->task_inst_);
			this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second.erase(this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second.find(*iter));
			cmd->ranked.insert(*iter);
		}
	}
	for(TaskInstSet::iterator iter=cmd->ranked.begin();iter!=cmd->ranked.end();iter++)
	{
		unranked->erase(unranked->find(*iter));
	}
	debug_text<<"max times for "<<cmd->task_inst_<<" : "<<cmd->new_start_max<<" "<<cmd->new_end_max<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	return true;
};

// Undo a command to adjust the maximum times of a
// task instance in the plan.
void SA_WorkingPlan::undo (SA_AdjustMaxTimesCmd *cmd)
{
	std::ostringstream debug_text;
	debug_text<<"task inst is "<<cmd->task_inst_<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	TimeWindow *start_win = &this->get_start_window(cmd->task_inst_);
	TimeWindow *end_win = &this->get_end_window(cmd->task_inst_);
	if(start_win->second==cmd->old_start_max && end_win->second==cmd->old_end_max) return;
	// Undo all the removal of the tasks from the unranked set
	TaskInstSet *unranked1 = &this->precedence_graph_.find(UNRANKED)->second.find(cmd->task_inst_)->second;
	TaskInstSet *after = &this->precedence_graph_.find(AFTER)->second.find(cmd->task_inst_)->second;
	for(TaskInstSet::iterator iter=cmd->ranked.begin();iter!=cmd->ranked.end();iter++)
	{
		TaskInstSet *unranked2 = &this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second;
		TaskInstSet *before = &this->precedence_graph_.find(BEFORE)->second.find(*iter)->second;
		unranked1->insert(*iter);
		unranked2->insert(cmd->task_inst_);
		after->erase(after->find(*iter));
		before->erase(before->find(cmd->task_inst_));
	}

	// Undo all the time window changes caused by this command
	for(SA_AdjustMaxTimesCmd::MaxTimesAdjustList::reverse_iterator iter=cmd->max_adjust_cmds.rbegin();iter!=cmd->max_adjust_cmds.rend();iter++)
	{
		this->undo(*iter);
	}

	// Reset the time window to the old one  
	start_win->second=cmd->old_start_max;
	end_win->second=cmd->old_end_max;

	return;
};
// Get the start window of the task instance. If one doesn't exist, then assign it one.
TimeWindow SA_WorkingPlan::get_start_window(TaskInstID task_inst)
{
	InstToWinMap::iterator iter = this->start_window_map_.find(task_inst);
	if(iter==this->start_window_map_.end()) {
		// The window doesn't exist
		// The default start window.
		TimeWindow temp_start = std::make_pair(0,NULL_TIME);
		TimeWindow temp_end;
		if(this->durations.find(task_inst)->second!=0)
		{
			// The default end window for non data tasks.
			temp_end = std::make_pair(this->durations.find(task_inst)->second,NULL_TIME);
		}
		else 
		{
			// The default end window for data tasks.		
			temp_end = std::make_pair(NULL_TIME,NULL_TIME);
		}
		this->start_window_map_.insert(std::make_pair(task_inst,temp_start));
		this->end_window_map_.insert(std::make_pair(task_inst,temp_end));
	}
	return this->start_window_map_.find(task_inst)->second;
};

TimeWindow SA_WorkingPlan::get_end_window(TaskInstID task_inst)
{
	InstToWinMap::iterator iter = this->end_window_map_.find(task_inst);
	if(iter==this->end_window_map_.end()) {
		// The window doesn't exist
		// The default start window.
		TimeWindow temp_start = std::make_pair(0,NULL_TIME);
		TimeWindow temp_end;
		if(this->durations.find(task_inst)->second!=0) 
		{
			// The default end window for non data tasks.
			temp_end = std::make_pair(this->durations.find(task_inst)->second,NULL_TIME);
		}
		else
		{
			// The default end window for data tasks.		
			temp_end = std::make_pair(NULL_TIME,NULL_TIME);
		}
		this->start_window_map_.insert(std::make_pair(task_inst,temp_start));
		this->end_window_map_.insert(std::make_pair(task_inst,temp_end));
	}
	return this->end_window_map_.find(task_inst)->second;
};

//Get the duration of a task instance
TimeValue SA_WorkingPlan::get_duration(TaskInstID task_inst)
{
	return this->durations.find(task_inst)->second;
};

/// Adds the sched order to the sched_links_ map by putting the first task instance before the second
void SA_WorkingPlan::add_sched_link(TaskInstID first_task_inst, TaskInstID second_task_inst)
{
	std::ostringstream debug_text;
	debug_text<<"Adding sched link insert"<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	this->sched_links_.insert(std::make_pair(first_task_inst,second_task_inst));
}
/// Removes the sched order from the sched_links_ map
void SA_WorkingPlan::remove_sched_link(TaskInstID first_task_inst, TaskInstID second_task_inst)
{
	for(SchedulingLinks::iterator iter=this->sched_links_.begin();iter!=this->sched_links_.end();iter++)
	{
		if(iter->first==first_task_inst && iter->second==second_task_inst)
		{
			this->sched_links_.erase(iter);
			break;
		}
	}
}
// Update the precedence graph by inserting the task instance.
bool SA_WorkingPlan::init_prec_insert(TaskInstID task_inst, SA_AssocTaskImplCmd *cmd)
{

	if(cmd->get_id().step == 12 && cmd->get_id().decision_pt== 2 && cmd->get_id().seq_num == 1 && task_inst == 3)
		bool yep = true;

	TaskInstSet temp;
	this->precedence_graph_.find(BEFORE)->second.insert(std::make_pair(task_inst,temp));
	this->precedence_graph_.find(AFTER)->second.insert(std::make_pair(task_inst,temp));
	this->precedence_graph_.find(UNRANKED)->second.insert(std::make_pair(task_inst,temp));
	this->precedence_graph_.find(SIMUL)->second.insert(std::make_pair(task_inst,temp));


	if(init_start.size()-1 <= task_inst){
		for(int i = init_start.size()-1; i < 100+task_inst; i++){
			this->init_start.insert(std::make_pair(i,(TimeWindow)std::make_pair(NULL_TIME,NULL_TIME)));
			this->init_end.insert(std::make_pair(i,(TimeWindow)std::make_pair(NULL_TIME,NULL_TIME)));
		}
	}

	TimeWindow win_start = this->init_start.find(task_inst)->second;
	TimeWindow win_end = this->init_end.find(task_inst)->second;
	TimeWindow start_win = this->get_start_window(task_inst);
	TimeWindow end_win = this->get_end_window(task_inst);


	std::ostringstream debug_text;
	debug_text<<"for "<<task_inst<<win_start.first<<" "<<win_start.second<<" "<<win_end.first<<" "<<win_end.second<<std::endl;
	debug_text<<"for "<<task_inst<<start_win.first<<" "<<start_win.second<<" "<<end_win.first<<" "<<end_win.second<<std::endl;
	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	if(win_start.first!=NULL_TIME)
	{
		// An initial time constraint is set on the minimum times
		if(win_start.second!=NULL_TIME)
		{
			// An initial time constraint is set on the maximum times
			cmd->min_adj_cmd.set_times(task_inst,win_start.first,win_end.first);
			cmd->max_adj_cmd.set_times(task_inst,win_start.second,win_end.second);
			if(!this->execute(&cmd->min_adj_cmd))
			{
				this->undo(&cmd->min_adj_cmd);
				return false;
			}
			if(!this->execute(&cmd->max_adj_cmd))
			{
				this->undo(&cmd->max_adj_cmd);
				this->undo(&cmd->min_adj_cmd);
				return false;
			}
		}
		else
		{
			// No initial time constraint is set on the maximum times
			cmd->min_adj_cmd.set_times(task_inst,win_start.first,win_end.first);
			if(!this->execute(&cmd->min_adj_cmd))
			{
				this->undo(&cmd->min_adj_cmd);
				return false;
			}
		}
	}
	else
	{
		// No initial time constraint is set on the minimum times
		if(win_start.second!=NULL_TIME)
		{
			// An initial time constraint is set on the maximum times
			cmd->max_adj_cmd.set_times(task_inst,win_start.second,win_end.second);
			if(!this->execute(&cmd->max_adj_cmd))
			{
				this->undo(&cmd->min_adj_cmd);
				return false;
			}
		}
		else
		{
			// No initial time constraint.
			cmd->min_adj_cmd.set_times(task_inst,start_win.first,end_win.first);
			cmd->max_adj_cmd.set_times(task_inst,start_win.second,end_win.second);

			std::ostringstream debug_text;
			debug_text<<"for "<<task_inst<<" "<<start_win.first<<" "<<start_win.second<<" "<<end_win.first<<" "<<end_win.second<<std::endl;
			SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
			debug_text.str("");


			if(!this->execute(&cmd->min_adj_cmd))
			{
				this->undo(&cmd->min_adj_cmd);
				return false;
			}
		}
	}
	start_win = this->get_start_window(task_inst);
	end_win = this->get_end_window(task_inst);
	TaskInstSet *before,*after,*simul,*unranked;
	before = &this->precedence_graph_.find(BEFORE)->second.find(task_inst)->second;
	after = &this->precedence_graph_.find(AFTER)->second.find(task_inst)->second;
	unranked = &this->precedence_graph_.find(UNRANKED)->second.find(task_inst)->second;
	simul = &this->precedence_graph_.find(SIMUL)->second.find(task_inst)->second;
	// If this task instance is not reused, insert all the task instances in it unranked set.
	if(unranked->empty() && (this->reused_insts_.find(task_inst)== this->reused_insts_.end()))
	{
		std::ostringstream debug_text;

		for(PrecedenceSet::iterator iter=this->precedence_graph_.find(BEFORE)->second.begin();iter!=this->precedence_graph_.find(BEFORE)->second.end();iter++)
		{
			if(iter->first!=task_inst)
			{
				debug_text<<"inserting "<<iter->first<<" in unranked of "<<task_inst<<std::endl;
				unranked->insert(iter->first);
			}
		}

		SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
		debug_text.str("");

		if(unranked->empty()) return true;
	}
	CLSet after_links = this->get_after(task_inst);

	TaskInstSet causal,data;
	for(CLSet::iterator iter=after_links.begin();iter!=after_links.end();iter++)
	{ 
		if(iter->second==GOAL_TASK_INST_ID || iter->second==NULL_TASK_INST_ID) continue;
		if(iter->cond.kind!=DATA) causal.insert(iter->second);
		else data.insert(iter->second);
	}

	for(TaskInstSet::iterator iter=causal.begin();iter!=causal.end();iter++)
	{
		TaskInstSet *iter_after = &this->precedence_graph_.find(AFTER)->second.find(*iter)->second;
		TaskInstSet *iter_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter)->second;
		TaskInstSet *iter_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second;
		TaskInstSet *iter_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter)->second;
		if(after->find(*iter)!=after->end()) continue;


		std::ostringstream debug_text;
		debug_text<<"inserting "<<*iter<<" in after set of "<<task_inst<<std::endl;
		TaskInstSet temp = *before;
		temp.insert(task_inst);
		for(TaskInstSet::iterator iter2=simul->begin();iter2!=simul->end();iter2++)
			temp.insert(*iter2);
		for(TaskInstSet::iterator iter2=temp.begin();iter2!=temp.end();iter2++)
		{
			TaskInstSet *iter2_before,*iter2_after,*iter2_simul,*iter2_unranked;
			iter2_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter2)->second;
			iter2_after = &this->precedence_graph_.find(AFTER)->second.find(*iter2)->second;
			iter2_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter2)->second;
			iter2_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter2)->second;
			for(TaskInstSet::iterator iter1=iter_after->begin();iter1!=iter_after->end();iter1++)
			{
				debug_text<<*iter1<<" is after "<<*iter<<" so it is after "<<*iter2<<std::endl;
				TaskInstSet *iter1_before,*iter1_unranked;
				iter1_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_before->find(*iter2)!=iter1_before->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter1));
				iter1_before->insert(*iter2);
				iter2_after->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}
			for(TaskInstSet::iterator iter1=iter_simul->begin();iter1!=iter_simul->end();iter1++)
			{
				debug_text<<*iter1<<" is siml to "<<*iter<<" so it is after "<<*iter2<<std::endl;
				TaskInstSet *iter1_before,*iter1_unranked;
				iter1_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_before->find(*iter2)!=iter1_before->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter1));
				iter1_before->insert(*iter2);
				iter2_after->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}
			if(iter_before->find(*iter2)!=iter_before->end()) continue;
			else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter));
			iter_before->insert(*iter2);
			iter2_after->insert(*iter);
			if(iter_unranked->find(*iter2)!=iter_unranked->end()) iter_unranked->erase(*iter2);
			if(iter2_unranked->find(*iter)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter));
		}

		SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
		debug_text.str("");

	}
	for(TaskInstSet::iterator iter=data.begin();iter!=data.end();iter++)
	{
		std::ostringstream debug_text;

		TaskInstSet *iter_after = &this->precedence_graph_.find(AFTER)->second.find(*iter)->second;
		TaskInstSet *iter_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter)->second;
		TaskInstSet *iter_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second;
		TaskInstSet *iter_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter)->second;
		if(simul->find(*iter)!=simul->end()) continue;
		debug_text<<"inserting "<<*iter<<" in simul set of "<<task_inst<<std::endl;
		TaskInstSet temp = *before;
		for(TaskInstSet::iterator iter2=temp.begin();iter2!=temp.end();iter2++)
		{
			debug_text<<"iter2 is "<<*iter2<<std::endl;
			TaskInstSet *iter2_before,*iter2_after,*iter2_simul,*iter2_unranked;
			iter2_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter2)->second;
			iter2_after = &this->precedence_graph_.find(AFTER)->second.find(*iter2)->second;
			iter2_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter2)->second;
			iter2_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter2)->second;
			for(TaskInstSet::iterator iter1=iter_after->begin();iter1!=iter_after->end();iter1++)
			{
				debug_text<<*iter1<<" is after "<<*iter<<" so it is after "<<*iter2<<std::endl;
				TaskInstSet *iter1_before,*iter1_unranked;
				iter1_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_before->find(*iter2)!=iter1_before->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter1));
				iter1_before->insert(*iter2);
				iter2_after->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}
			for(TaskInstSet::iterator iter1=iter_simul->begin();iter1!=iter_simul->end();iter1++)
			{
				debug_text<<*iter1<<" is simul to "<<*iter<<" so it is after "<<*iter2<<std::endl;
				TaskInstSet *iter1_before,*iter1_unranked;
				iter1_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_before->find(*iter2)!=iter1_before->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter1));
				iter1_before->insert(*iter2);
				iter2_after->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}    
			if(iter_before->find(*iter2)!=iter_before->end()) continue;
			else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter));
			iter_before->insert(*iter2);
			iter2_after->insert(*iter);
			if(iter_unranked->find(*iter2)!=iter_unranked->end()) iter_unranked->erase(*iter2);
			if(iter2_unranked->find(*iter)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter));

			SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
			debug_text.str("");
		}


		temp = *simul;
		temp.insert(task_inst);
		for(TaskInstSet::iterator iter2=temp.begin();iter2!=temp.end();iter2++)
		{
			std::ostringstream debug_text;

			TaskInstSet *iter2_before,*iter2_after,*iter2_simul,*iter2_unranked;
			iter2_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter2)->second;
			iter2_after = &this->precedence_graph_.find(AFTER)->second.find(*iter2)->second;
			iter2_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter2)->second;
			iter2_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter2)->second;
			for(TaskInstSet::iterator iter1=iter_after->begin();iter1!=iter_after->end();iter1++)
			{
				debug_text<<*iter1<<" is after "<<*iter<<" so it is after "<<*iter2<<std::endl;
				TaskInstSet *iter1_before,*iter1_unranked;
				iter1_before = &this->precedence_graph_.find(BEFORE)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_before->find(*iter2)!=iter1_before->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter2,*iter1));
				iter1_before->insert(*iter2);
				iter2_after->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}
			for(TaskInstSet::iterator iter1=iter_simul->begin();iter1!=iter_simul->end();iter1++)
			{
				debug_text<<*iter1<<" is simul "<<*iter<<" so it is simul "<<*iter2<<std::endl;
				TaskInstSet *iter1_simul,*iter1_unranked;
				iter1_simul = &this->precedence_graph_.find(SIMUL)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_simul->find(*iter2)!=iter1_simul->end()) continue;
				else 
				{
					cmd->simul_insertions.insert(std::make_pair(*iter2,*iter1));
				}
				iter1_simul->insert(*iter2);
				iter2_simul->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}
			for(TaskInstSet::iterator iter1=iter_before->begin();iter1!=iter_before->end();iter1++)
			{
				debug_text<<*iter1<<" is before "<<*iter<<" so it is before "<<*iter2<<std::endl;
				TaskInstSet *iter1_after,*iter1_unranked;
				iter1_after = &this->precedence_graph_.find(AFTER)->second.find(*iter1)->second;
				iter1_unranked = &this->precedence_graph_.find(UNRANKED)->second.find(*iter1)->second;
				if(iter1_after->find(*iter2)!=iter1_after->end()) continue;
				else cmd->causal_insertions.insert(std::make_pair(*iter1,*iter2));
				iter1_after->insert(*iter2);
				iter2_before->insert(*iter1);
				if(iter1_unranked->find(*iter2)!=iter1_unranked->end()) iter1_unranked->erase(*iter2);
				if(iter2_unranked->find(*iter1)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter1));
			}

			SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
			debug_text.str("");

			if(iter_simul->find(*iter2)!=iter_simul->end()) continue;
			else cmd->simul_insertions.insert(std::make_pair(*iter2,*iter));
			iter2_simul->insert(*iter);
			iter_simul->insert(*iter2);
			if(iter_unranked->find(*iter2)!=iter_unranked->end()) iter_unranked->erase(*iter2);
			if(iter2_unranked->find(*iter)!=iter2_unranked->end()) iter2_unranked->erase(iter2_unranked->find(*iter));
		}
	}


	debug_text<<"for "<<task_inst<<" start window: "<<start_win.first<<"-"<<start_win.second<<" and end window "<<end_win.first<<"-"<<end_win.second<<std::endl;


	for(TaskInstSet::iterator iter = unranked->begin();iter!=unranked->end();iter++)
	{
		TimeWindow temp_start = this->get_start_window(*iter);
		TimeWindow temp_end = this->get_end_window(*iter);
		debug_text<<"checking "<<*iter<<" in all with start window: "<<temp_start.first<<"-"<<temp_start.second<<" and end window "<<temp_end.first<<"-"<<temp_end.second<<std::endl;
		if(end_win.second!=NULL_TIME && temp_start.first>end_win.second)
		{
			after->insert(*iter);
			this->precedence_graph_.find(BEFORE)->second.find(*iter)->second.insert(task_inst);
		}
		else if(temp_end.second!=NULL_TIME && temp_end.second<start_win.first)
		{
			before->insert(*iter);
			this->precedence_graph_.find(AFTER)->second.find(*iter)->second.insert(task_inst);
		}
		else
		{
			this->precedence_graph_.find(UNRANKED)->second.find(*iter)->second.insert(task_inst);
		}


	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

	this->print_prec();

	for(InstToWinMap::iterator iter=this->start_window_map_.begin();iter!=this->start_window_map_.end();iter++)
	{
		debug_text<<"The start window for "<<iter->first<<" is "<<iter->second.first<<" - "<<iter->second.second<<std::endl;
	}
	for(InstToWinMap::iterator iter=this->end_window_map_.begin();iter!=this->end_window_map_.end();iter++)
	{
		debug_text<<"The end window for "<<iter->first<<" is "<<iter->second.first<<" - "<<iter->second.second<<std::endl;
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_NORMAL, debug_text.str ());
	debug_text.str("");

	return true;
};

// Print the precedence graph
void SA_WorkingPlan::print_prec()
{
	std::ostringstream debug_text;

	debug_text<<"BEFORE : "<<BEFORE<<", AFTER : "<<AFTER<<", SIMUL : "<<SIMUL<<", UNRANKED : "<<UNRANKED<<std::endl;
	for(PrecedenceGraph::iterator iter1 = this->precedence_graph_.begin(); iter1 != this->precedence_graph_.end(); iter1++)
	{
		debug_text<<"The precedence set "<<iter1->first<<": "<<std::endl;
		for(PrecedenceSet::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++)
		{
			debug_text<<"The task inst set of "<<iter2->first<<" is ";
			for(TaskInstSet::iterator iter3 = iter2->second.begin(); iter3 != iter2->second.end(); iter3++)
			{
				debug_text<<*iter3<<",";
			}
			debug_text<<std::endl;
		}
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");
};

// Undo all the updation to the precedence graph due to the chosen task instance
void SA_WorkingPlan::prec_erase(TaskInstID task_inst, SA_AssocTaskImplCmd *cmd)
{
	PrecedenceSet *before = &this->precedence_graph_.find(BEFORE)->second;
	PrecedenceSet *simul = &this->precedence_graph_.find(SIMUL)->second;
	PrecedenceSet *after = &this->precedence_graph_.find(AFTER)->second;
	PrecedenceSet *unranked = &this->precedence_graph_.find(UNRANKED)->second;

	std::ostringstream debug_text;

	for(std::set< std::pair<TaskInstID,TaskInstID> >::iterator iter = cmd->causal_insertions.begin();iter!=cmd->causal_insertions.end();iter++)
	{
		// iter->second has been inserted in the after set of iter->first
		debug_text<<"removing "<<iter->second<<" from after of "<<iter->first<<std::endl;
		before->find(iter->second)->second.erase(iter->first);
		after->find(iter->first)->second.erase(iter->second);
		unranked->find(iter->first)->second.insert(iter->second);
		unranked->find(iter->second)->second.insert(iter->first);
	}
	for(std::set< std::pair<TaskInstID,TaskInstID> >::iterator iter = cmd->simul_insertions.begin();iter!=cmd->simul_insertions.end();iter++)
	{
		// iter->second has been inserted in the simul set of iter->first
		debug_text<<"removing "<<iter->second<<" from simul of "<<iter->first<<std::endl;
		simul->find(iter->second)->second.erase(iter->first);
		simul->find(iter->first)->second.erase(iter->second);
		unranked->find(iter->first)->second.insert(iter->second);
		unranked->find(iter->second)->second.insert(iter->first);
	}

	debug_text<<this->reused_insts_.size()<<std::endl;
	if(this->reused_insts_.find(task_inst)==this->reused_insts_.end())
	{
		before->erase(before->find(task_inst));
		after->erase(after->find(task_inst));
		simul->erase(simul->find(task_inst));
		unranked->erase(unranked->find(task_inst));
		this->start_window_map_.erase(this->start_window_map_.find(task_inst));
		this->end_window_map_.erase(this->end_window_map_.find(task_inst));
		for(PrecedenceSet::iterator iter1 = unranked->begin();iter1!=unranked->end();iter1++)
			iter1->second.erase(task_inst);
	}

	SA_POP_DEBUG_STR (SA_POP_DEBUG_VERBOSE, debug_text.str ());
	debug_text.str("");

};
/// Get all the task instances
TaskInstSet SA_WorkingPlan::get_all_insts()
{
	TaskInstSet all;
	for(PrecedenceSet::iterator iter=this->precedence_graph_.find(BEFORE)->second.begin();iter!=this->precedence_graph_.find(BEFORE)->second.end();iter++)
	{
		all.insert(iter->first);
	}	  
	return all;
}
/// Check if the instance id already exists and is being reused.
bool SA_WorkingPlan::inst_exists (TaskInstID task_inst)
{

	if(this->task_impls_.find(task_inst)!=this->task_impls_.end()) return true;
	else return false;
}

/// Get task implementation for a task instance.
TaskImplID SA_WorkingPlan::get_impl_id (TaskInstID task_inst)
{
	return this->task_impls_.find(task_inst)->second;
}

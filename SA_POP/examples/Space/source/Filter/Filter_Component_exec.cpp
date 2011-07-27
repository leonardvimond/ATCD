#include "Filter_Component_exec.h"
#include "ciao/CIAO_common.h"

namespace CIDL_Filter_Component
{
  //==================================================================
  // Component Executor Implementation Class:   Filter_Component_exec_i
  //==================================================================

  Filter_Component_exec_i::Filter_Component_exec_i (void)
  {
  }

  Filter_Component_exec_i::~Filter_Component_exec_i (void)
  {
  }

  // Supported or inherited operations.

  // Attribute operations.

  // Port operations.

  void
  Filter_Component_exec_i::push_recv_data (
    ::SPACE::Gizmo_Data * ev)
  ACE_THROW_SPEC (( ::CORBA::SystemException))
  {

    ACE_DEBUG ((LM_DEBUG, "(%P|%t) Filter-Component: "
                "Got science data from %s.\n", ev->sender ()));
    this->context_->push_send_data (ev);
    // Your code here.
  }

  // Operations from Components::SessionComponent

  void
  Filter_Component_exec_i::set_session_context (
    ::Components::SessionContext_ptr ctx
    ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    this->context_ =
      Filter_Component_Context::_narrow (
        ctx
        ACE_ENV_ARG_PARAMETER);
    ACE_CHECK;

    if (this->context_ == 0)
    {
      ACE_THROW ( ::CORBA::INTERNAL ());
    }
  }

  void
  Filter_Component_exec_i::ciao_preactivate (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    // Your code here.
  }

  void
  Filter_Component_exec_i::configuration_complete (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    // Your code here.
  }

  void
  Filter_Component_exec_i::ccm_activate (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    // Your code here.
  }

  void
  Filter_Component_exec_i::ccm_passivate (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    // Your code here.
  }

  void
  Filter_Component_exec_i::ccm_remove (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    // Your code here.
  }

  //==================================================================
  // Home Executor Implementation Class:   Filter_Component_Factory_exec_i
  //==================================================================

  Filter_Component_Factory_exec_i::Filter_Component_Factory_exec_i (void)
  {
  }

  Filter_Component_Factory_exec_i::~Filter_Component_Factory_exec_i (void)
  {
  }

  // Supported or inherited operations.

  // Home operations.

  // Factory and finder operations.

  // Attribute operations.

  // Implicit operations.

  ::Components::EnterpriseComponent_ptr
  Filter_Component_Factory_exec_i::create (
    ACE_ENV_SINGLE_ARG_DECL)
  ACE_THROW_SPEC (( ::CORBA::SystemException,
                   ::Components::CCMException))
  {
    ::Components::EnterpriseComponent_ptr retval =
      ::Components::EnterpriseComponent::_nil ();

    ACE_NEW_THROW_EX (
      retval,
      Filter_Component_exec_i,
      ::CORBA::NO_MEMORY ());
    ACE_CHECK_RETURN (::Components::EnterpriseComponent::_nil ());

    return retval;
  }

  extern "C" FILTER_COMPONENT_EXEC_Export ::Components::HomeExecutorBase_ptr
  create_SPACE_Filter_Component_Factory_Impl (void)
  {
    ::Components::HomeExecutorBase_ptr retval =
      ::Components::HomeExecutorBase::_nil ();

    ACE_NEW_RETURN (
      retval,
      Filter_Component_Factory_exec_i,
      ::Components::HomeExecutorBase::_nil ());

    return retval;
  }
}
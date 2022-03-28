#include "K2Node_CustomNode.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Self.h"
#include "KismetCompiler.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node_CustomNode"

const FName UK2Node_CustomNode::InClassPin = FName("InClassPin");
const FName UK2Node_CustomNode::OutStringPin = FName("OutStringPin");
const FName UK2Node_CustomNode::OutDelegatePin = FName("OutDelegatePin");
const FName UK2Node_CustomNode::OnCastFailedPin = FName("OnCastFailedPin");

void UK2Node_CustomNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* NodeClass = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(NodeClass))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(NodeClass, NodeSpawner);
	}
}

FText UK2Node_CustomNode::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "사용자 설정 블루프린트 노드");
}

FText UK2Node_CustomNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeName", "새로 만든 커스텀 노드");
}

FText UK2Node_CustomNode::GetTooltipText() const
{
	return LOCTEXT("CustomToolTip", "사용자가 커스터마이징한 기능을 갖는 노드입니다.");
}

void UK2Node_CustomNode::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, AActor::StaticClass(),
		InClassPin);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	/*CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Delegate,
		StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UK2Node_CustomNode, DelegateSignatureInfo)),
			OutDelegatePin);*/
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, OnCastFailedPin);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_String, OutStringPin);
}

void UK2Node_CustomNode::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode( CompilerContext, SourceGraph );

	if (CheckAnyErrors(CompilerContext))
	{
		BreakAllNodeLinks();
		return;
	}
	
	//Intermediate 노드 스폰
	UK2Node_IfThenElse* IfThenElseNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	IfThenElseNode->AllocateDefaultPins();

	UK2Node_DynamicCast* DynamicCastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, SourceGraph);
	DynamicCastNode->TargetType = Cast<UClass>(GetInClassPin()->DefaultObject);
	DynamicCastNode->SetPurity(true);
	DynamicCastNode->AllocateDefaultPins();

	UK2Node_CallFunction* CallFuncIsValidNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	const FName IsValidFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid);
	CallFuncIsValidNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
	CallFuncIsValidNode->AllocateDefaultPins();

	UK2Node_CallFunction* CallFuncGetObjNameNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	const FName GetObjNameFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, GetObjectName);
	CallFuncGetObjNameNode->FunctionReference.SetExternalMember(GetObjNameFuncName, UKismetSystemLibrary::StaticClass());
	CallFuncGetObjNameNode->AllocateDefaultPins();

	UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
	SelfNode->AllocateDefaultPins();

	// 핀 정리 작업 (모든 핀 끌어오기)
	UEdGraphPin* This_ExecPin = GetExecPin();
	UEdGraphPin* This_InClassPin = GetInClassPin();
	UEdGraphPin* This_ThenPin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* This_OnCastFailedPin = GetOnCastFailedPin();
	UEdGraphPin* This_OutStringPin = GetOutStringPin();
	
	UEdGraphPin* IfThenElse_ExecPin = IfThenElseNode->GetExecPin();
	UEdGraphPin* IfThenElse_ConditionPin = IfThenElseNode->GetConditionPin();
	UEdGraphPin* IfThenElse_ThenPin = IfThenElseNode->GetThenPin();
	UEdGraphPin* IfThenElse_ElsePin = IfThenElseNode->GetElsePin();

	UEdGraphPin* DynamicCast_CastSrcPin = DynamicCastNode->GetCastSourcePin();
	UEdGraphPin* DynamicCast_CastResPin = DynamicCastNode->GetCastResultPin();

	UEdGraphPin* CallFuncIsValid_InObjPin = CallFuncIsValidNode->FindPinChecked(FName("Object"));
	UEdGraphPin* CallFuncIsValid_OutBooleanPin = CallFuncIsValidNode->GetReturnValuePin();

	UEdGraphPin* CallFuncGetObjName_InObjectPin = CallFuncGetObjNameNode->FindPinChecked(FName("Object"));
	UEdGraphPin* CallFuncGetObjName_OutStringPin = CallFuncGetObjNameNode->GetReturnValuePin();

	UEdGraphPin* SelfPin = SelfNode->FindPinChecked(UEdGraphSchema_K2::PN_Self);

	//핀끼리 연결.
	const UEdGraphSchema_K2* Schema = Cast<UEdGraphSchema_K2>(GetSchema());
	check(Schema);

	bool ConnectionError = false;
	
	ConnectionError |= !CompilerContext.MovePinLinksToIntermediate(*This_ExecPin, *IfThenElse_ExecPin).CanSafeConnect();
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError0", "Node @@, This_ExecPin to DynamicCast_ExecPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}

	ConnectionError |= !Schema->TryCreateConnection(SelfPin, DynamicCast_CastSrcPin);
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError1", "Node @@, SelfPin to DynamicCast_CastSrcPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}
	
	ConnectionError |= !Schema->TryCreateConnection(DynamicCast_CastResPin, CallFuncIsValid_InObjPin);
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError2", "Node @@, DynamicCast_CastResPin to CallFuncIsValid_InObjPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}

	ConnectionError |= !Schema->TryCreateConnection(CallFuncIsValid_OutBooleanPin, IfThenElse_ConditionPin);
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError3", "Node @@, This_ExecPin to DynamicCast_ExecPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}

	ConnectionError |= !Schema->TryCreateConnection(DynamicCast_CastResPin, CallFuncGetObjName_InObjectPin);
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError4", "Node @@, DynamicCast_CastResPin to CallFuncGetObjName_InObjectPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}

	ConnectionError |= !CompilerContext.MovePinLinksToIntermediate(*This_ThenPin, *IfThenElse_ThenPin).CanSafeConnect();
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError5", "Node @@, IfThenElse_ThenPin to This_ThenPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}
	
	ConnectionError |= !CompilerContext.MovePinLinksToIntermediate(*This_OnCastFailedPin, *IfThenElse_ElsePin).CanSafeConnect();
	if(ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError6", "Node @@, IfThenElse_ElsePin to This_OnCastFailedPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}
	
	ConnectionError |= !CompilerContext.MovePinLinksToIntermediate(*This_OutStringPin, *CallFuncGetObjName_OutStringPin).CanSafeConnect();
	if (ConnectionError)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT( "ConnectionError7", "Node @@, CallFuncGetObjName_OutStringPin to This_OutStringPin." ).ToString( ), this);
		BreakAllNodeLinks();
		return;
	}

	BreakAllNodeLinks( );
}

bool UK2Node_CustomNode::CheckAnyErrors(FKismetCompilerContext& CompilerContext)
{
	bool bErrors = false;
	if (GetInClassPin()->DefaultObject == nullptr)
	{
		CompilerContext.MessageLog.Error( *LOCTEXT( "Error", "Node @@ had an input error." ).ToString( ), this );
		//CompilerContext.MessageLog.Warning(*LOCTEXT( "Warning", "Node @@ had an input Warning." ).ToString( ), this);
		bErrors = true;
	}

	return bErrors;
}

#undef LOCTEXT_NAMESPACE



	

	
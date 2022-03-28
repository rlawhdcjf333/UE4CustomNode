#pragma once

#include "CoreMinimal.h"

#include "K2Node.h"
#include "K2Node_CustomNode.generated.h"

UCLASS()
class CUSTOMNODEMODULE_API UK2Node_CustomNode : public UK2Node
{
	GENERATED_BODY()

public:
	virtual bool IsNodeSafeToIgnore() const override {return true;}

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;

	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	void DelegateSignatureInfo() {}

	bool CheckAnyErrors(FKismetCompilerContext& CompilerContext);

private:
	static const FName InClassPin;
	static const FName OutStringPin;
	static const FName OutDelegatePin;
	static const FName OnCastFailedPin;

public:
	/* Pin Parameter Accessors	*/
	UEdGraphPin* GetInClassPin() const {return FindPinChecked(InClassPin);}
	UEdGraphPin* GetOutStringPin() const {return FindPinChecked(OutStringPin);}
	UEdGraphPin* GetOutDelegatePin() const {return FindPinChecked(OutDelegatePin);}
	UEdGraphPin* GetOnCastFailedPin() const {return FindPinChecked(OnCastFailedPin);}
};


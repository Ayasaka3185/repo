#include "Dialogue/DialogueComponent.h"

#include "Blueprint/UserWidget.h"
#include "Dialogue/DialogueWidget.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	BuildDemoDialogueIfEmpty();
	CacheInlineNodes();
}

void UDialogueComponent::BuildDemoDialogueIfEmpty()
{
	if (DialogueTable || InlineNodes.Num() > 0)
	{
		return;
	}

	FUIDialogueNode Start;
	Start.NodeId = TEXT("Start");
	Start.SpeakerName = FText::FromString(TEXT("测试 NPC"));
	Start.DialogueText = FText::FromString(TEXT("我是一个对话npc"));
	Start.Choices = {
		{FText::FromString(TEXT("我看到了，继续说明。")), TEXT("Explain")},
		{FText::FromString(TEXT("这个系统能做复杂分支吗？")), TEXT("Branch")},
		{FText::FromString(TEXT("先结束对话。")), TEXT("End")}
	};

	FUIDialogueNode Explain;
	Explain.NodeId = TEXT("Explain");
	Explain.SpeakerName = FText::FromString(TEXT("测试 NPC"));
	Explain.DialogueText = FText::FromString(TEXT("只需要替换模型，再挂同一个 DialogueComponent，就能复用这套 UI。"));
	Explain.Choices = {
		{FText::FromString(TEXT("返回上一个问题。")), TEXT("Start")},
		{FText::FromString(TEXT("继续问复杂分支。")), TEXT("Branch")},
		{FText::FromString(TEXT("结束。")), TEXT("End")}
	};

	FUIDialogueNode Branch;
	Branch.NodeId = TEXT("Branch");
	Branch.SpeakerName = FText::FromString(TEXT("测试 NPC"));
	Branch.DialogueText = FText::FromString(TEXT("可以。每个节点都有自己的 NextNodeId，选项也能指向任意节点。"));
	Branch.Choices = {
		{FText::FromString(TEXT("走到更深一层。")), TEXT("DeepA")},
		{FText::FromString(TEXT("跳到另一条分支。")), TEXT("DeepB")},
		{FText::FromString(TEXT("回到开始。")), TEXT("Start")}
	};

	FUIDialogueNode DeepA;
	DeepA.NodeId = TEXT("DeepA");
	DeepA.SpeakerName = FText::FromString(TEXT("测试 NPC"));
	DeepA.DialogueText = FText::FromString(TEXT("这是 A 分支的下一层。再复杂也只是继续填 NextNodeId。"));
	DeepA.NextNodeId = TEXT("Branch");

	FUIDialogueNode DeepB;
	DeepB.NodeId = TEXT("DeepB");
	DeepB.SpeakerName = FText::FromString(TEXT("测试 NPC"));
	DeepB.DialogueText = FText::FromString(TEXT("这是 B 分支。它也可以回到任何旧节点，不限制层数。"));
	DeepB.NextNodeId = TEXT("Start");

	InlineNodes = {Start, Explain, Branch, DeepA, DeepB};
}

void UDialogueComponent::CacheInlineNodes()
{
	NodeMap.Reset();

	for (const FUIDialogueNode& Node : InlineNodes)
	{
		if (!Node.NodeId.IsNone())
		{
			NodeMap.Add(Node.NodeId, Node);
		}
	}

	if (DialogueTable)
	{
		TArray<FUIDialogueTableRow*> Rows;
		DialogueTable->GetAllRows<FUIDialogueTableRow>(TEXT("DialogueComponent"), Rows);
		for (const FUIDialogueTableRow* Row : Rows)
		{
			if (Row && !Row->Node.NodeId.IsNone())
			{
				NodeMap.Add(Row->Node.NodeId, Row->Node);
			}
		}
	}
}

void UDialogueComponent::StartDialogue(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	CacheInlineNodes();

	FUIDialogueNode FirstNode;
	if (!GetNode(StartNodeId, FirstNode))
	{
		return;
	}

	ActiveDialogueWidget = CreateWidget<UDialogueWidget>(PlayerController, UDialogueWidget::StaticClass());
	if (!ActiveDialogueWidget)
	{
		return;
	}

	ActivePlayerController = PlayerController;
	PlayerController->SetShowMouseCursor(true);
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ActiveDialogueWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);

	ActiveDialogueWidget->AddToViewport(80);
	ActiveDialogueWidget->StartDialogue(this, FirstNode);
}

bool UDialogueComponent::GetNode(FName NodeId, FUIDialogueNode& OutNode) const
{
	if (const FUIDialogueNode* FoundNode = NodeMap.Find(NodeId))
	{
		OutNode = *FoundNode;
		return true;
	}
	return false;
}

void UDialogueComponent::AdvanceToNode(FName NodeId)
{
	if (!ActiveDialogueWidget)
	{
		return;
	}

	if (NodeId.IsNone() || NodeId == TEXT("End"))
	{
		EndDialogue();
		return;
	}

	FUIDialogueNode NextNode;
	if (GetNode(NodeId, NextNode))
	{
		ActiveDialogueWidget->ShowNode(NextNode);
	}
	else
	{
		EndDialogue();
	}
}

void UDialogueComponent::EndDialogue()
{
	if (ActiveDialogueWidget)
	{
		ActiveDialogueWidget->RemoveFromParent();
		ActiveDialogueWidget = nullptr;
	}

	if (ActivePlayerController)
	{
		ActivePlayerController->SetShowMouseCursor(false);
		ActivePlayerController->SetInputMode(FInputModeGameOnly());
		ActivePlayerController = nullptr;
	}
}

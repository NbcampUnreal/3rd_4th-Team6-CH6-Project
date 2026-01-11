#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SFCreateRoomWidget.generated.h"

class USFOSSGameInstance;
class UCheckBox;
class UButton;
class UCommonButtonBase;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class SF_API USFCreateRoomWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    //====================================UI 위젯======================================
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* RoomNameInput; //방 이름 입력란

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* PasswordInput; //비밀번호 입력란

    UPROPERTY(meta = (BindWidget))
    UCommonButtonBase* CreateButton; //생성 버튼

    UPROPERTY(meta = (BindWidget))
    UCommonButtonBase* CancelButton; //취소 버튼

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ErrorMessageText; //오류 메시지 표시

    UPROPERTY(meta = (BindWidget))
    UCheckBox* SecretRoomCheckBox; //비밀방 여부 체크박스

    // 커스텀 카운터 플레이어 카운트 UI
    UPROPERTY(meta = (BindWidget))
    UButton* DecreasePlayerCountButton; // [-] 버튼

    UPROPERTY(meta = (BindWidget))
    UButton* IncreasePlayerCountButton; // [+] 버튼

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MaxPlayersText;

    // 현재 선택된 인원 수를 저장할 변수
    int32 CurrentMaxPlayerCount = 1;
    
    //==================================================================================

    //====================================콜백 바인딩=====================================
    UFUNCTION()
    void OnCreateButtonClicked(); //방 생성 버튼 클릭 처리

    UFUNCTION()
    void OnCancelButtonClicked(); //방 생성 취소 처리

    //  버튼 클릭 함수
    UFUNCTION()
    void OnDecreasePlayerCountClicked();

    UFUNCTION()
    void OnIncreasePlayerCountClicked();
    
    // UI 업데이트 헬퍼 함수
    void UpdateMaxPlayerDisplay();

    UFUNCTION()
    void OnSecretRoomCheckboxChanged(bool bIsChecked); //비밀번호 입력 활성 여부

    UFUNCTION()
    void OnCreateSessionComplete(bool bWasSuccessful, const FString& Message); //세션 생성 완료 콜백
    //===================================================================================

private:
    UPROPERTY()
    USFOSSGameInstance* GameInstance; //세션 생성, 관리용 GameInstance 참조
};
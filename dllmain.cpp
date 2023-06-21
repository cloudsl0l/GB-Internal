#include "SDK.hpp"
#include <vector>

#define RELATIVE(addr, size) ((uintptr_t)((UINT_PTR)(addr) + *(PINT)((UINT_PTR)(addr) + ((size) - sizeof(INT))) + (size)))

void MainRender(SDK::UObject* object, SDK::UCanvas* Canvas) {

	if (Canvas) {

		if (!ue->SetObjects()) return;

		ue->PlayerController->GetViewportSize(&x, &y);

		Canvas->K2_DrawBox(SDK::FVector2D(100, 100), SDK::FVector2D(30, 30), 1.f, SDK::FLinearColor(255, 255, 255, 255));
		Canvas->K2_DrawLine(SDK::FVector2D(100, 100), SDK::FVector2D(200, 200), 1.f, SDK::FLinearColor(255, 255, 255, 255));

		auto MyPawn = ue->World->OwningGameInstance->LocalPlayers[0]->PlayerController->Pawn;
		auto MyPlayer = ue->PlayerController->K2_GetPawn();
		auto CamManager = ue->PlayerController->PlayerCameraManager;
		auto MyLoc = ue->PlayerController->PlayerCameraManager->GetCameraLocation();
		auto MyRot = ue->PlayerController->PlayerCameraManager->GetCameraRotation();

		SDK::AGBCharacter* closest_player = 0;
		float closest_distance = FLT_MAX;

		SDK::FVector2D wnd_size = SDK::FVector2D(x, y);

		SDK::TArray<SDK::AActor*> projectile;
		SDK::TArray<SDK::AActor*> actors;
		//ABP_MainPlayerCharacter_C
		//ABP_PlayerCharacter_C
		ue->GPStatics->STATIC_GetAllActorsOfClass(ue->World, (SDK::AActor*)SDK::AGBCharacter::StaticClass(), &actors);

		ue->GPStatics->STATIC_GetAllActorsOfClass(ue->World, (SDK::AActor*)SDK::AGBBulletProjectile::StaticClass(), &projectile);

		for (int i = 0; i < actors.Num(); i++) {

			SDK::AGBCharacter* enemies = (SDK::AGBCharacter*)actors[i];

			if (enemies)
			{

				auto EnemyMesh = enemies->Mesh;

				auto color = (true) ? SDK::FLinearColor(0.f, 255.f, 0.f, 255.f) : SDK::FLinearColor(217.f, 17.f, 0.f, 255.f);

				if (!enemies->IsAlive()) continue;

				SDK::FVector ActorLoc = enemies->K2_GetActorLocation();

				

				SDK::FVector2D ScreenLoc;

				std::vector<std::pair<int, int>> SkeletonConnections = {
				{neck_01, Head},
				{spine_02, neck_01},
				{spine_01, spine_02},
				{spine_03, spine_01}, // stomach to chest
				{pelvis, spine_03},
				{clavicle_r, spine_02},
				{upperarm_r, clavicle_r},
				{lowerarm_r, upperarm_r}, // right elbow
				{hand_r, lowerarm_r},
				{clavicle_l, spine_02},
				{upperarm_l, clavicle_l},
				{lowerarm_l, upperarm_l}, // left elbow
				{hand_l, lowerarm_l},
				{thigh_r, pelvis},
				{calf_r, thigh_r},
				{foot_r, calf_r},
				{thigh_l, pelvis},
				{calf_l, thigh_l},
				{foot_l, calf_l},
				};

				SDK::FVector2D BoneScreen, PrevBoneScreen;
				for (const std::pair<int, int>& Connection : SkeletonConnections)
				{
					const auto Bone1 = Connection.first;
					const auto Bone2 = Connection.second;

					const auto BoneLoc1 = enemies->Mesh->GetSocketLocation(enemies->Mesh->GetBoneName(Bone1));
					const auto BoneLoc2 = enemies->Mesh->GetSocketLocation(enemies->Mesh->GetBoneName(Bone2));

					const auto Screen1 = ue->PlayerController->ProjectWorldLocationToScreen(BoneLoc1, false, &BoneScreen);
					const auto Screen2 = ue->PlayerController->ProjectWorldLocationToScreen(BoneLoc2, false, &PrevBoneScreen);

					if (Screen1 && Screen2)
						Canvas->K2_DrawLine(BoneScreen, PrevBoneScreen, 1.5f, SDK::FLinearColor(255, 255, 255, 255));

				}

				const auto BoneLoc1 = enemies->Mesh->GetSocketLocation(enemies->Mesh->GetBoneName(Head));
				const auto BoneLoc2 = enemies->Mesh->GetSocketLocation(enemies->Mesh->GetBoneName(foot_l));

				SDK::FVector WorldHead = BoneLoc1;
				SDK::FVector WorldBottom = BoneLoc2;

				SDK::FVector2D head; ue->PlayerController->ProjectWorldLocationToScreen(WorldHead, false, &head);
				SDK::FVector2D Top; ue->PlayerController->ProjectWorldLocationToScreen({ BoneLoc1.X, BoneLoc1.Y, BoneLoc1.Z + 20 }, false, &Top);
				SDK::FVector2D Bottom; ue->PlayerController->ProjectWorldLocationToScreen(WorldBottom, false, &Bottom);

				ue->Box2D(Canvas, enemies, Bottom, Top, color);

				
				auto dx = head.X - (x / 2);									
				auto dy = head.Y - (y / 2);									
				auto dist = sqrtf(dx * dx + dy * dy);						
				
				if (dist < closest_distance) {								
					closest_distance = dist;								
					closest_player = enemies;								
				}															

				//DrawCircle(Canvas, x / 2, y / 2, 120, 30, SDK::FLinearColor(255, 255, 255, 255)); // Draw FOV circle

				SDK::FVector TpLoc = closest_player->K2_GetActorLocation();

				if (GetAsyncKeyState(VK_F1))
					MyPlayer->K2_TeleportTo(TpLoc, MyRot);

				if (enemies->IsLocallyControlled()) {
					enemies->SetNoRecoil(true);
					enemies->SetNoSway(true);
				}

			}

		}

		for (int i = 0; i < projectile.Num(); i++) {

			SDK::AGBBulletProjectile* bullets = (SDK::AGBBulletProjectile*)projectile[i];

			if (bullets) {

				SDK::FVector TpLoc = closest_player->Mesh->GetSocketLocation(closest_player->Mesh->GetBoneName(Head));
				SDK::FHitResult HResult;

				bool teleport = false;

				if (GetAsyncKeyState(VK_CAPITAL))
					teleport = true;

				bool isVisible = closest_player->Mesh->IsVisible();

				if (isVisible && teleport)
					bullets->K2_SetActorLocation(TpLoc, false, true, &HResult);

			}

		}

		if (GetAsyncKeyState(VK_TAB))
			ue->SetAim(CamManager, closest_player);

	}

	return origin_renderer(object, Canvas);
}

void main() {

	/***************-UWORLD-**********************/
	ue->GWorld = (SDK::UWorld**)((DWORD64)GetModuleHandleW(NULL) + 0x650aaa0);
	/********************************************/
	uintptr_t GWorld;

	const auto address4 = FindPattern(GetModuleHandleW(NULL),
		reinterpret_cast<const unsigned char*>("\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\xC8\xE8\x00\x00\x00\x00\x84"),
		"xxx????xxxx????x");

	GWorld = RELATIVE(address4, 7);

	printf("GWorld: 0x%x\n", GWorld - (uintptr_t)GetModuleHandleW(0));

	uintptr_t GlobalObjects;

	const auto address = FindPattern(GetModuleHandleW(NULL),
		reinterpret_cast<const unsigned char*>("\x48\x8B\x05\x00\x00\x0\x00\x48\x8B\x0C\x07\x48\x85\xC9\x74\x20"),
		"xxx????xxxxxxxxx");

	const auto offset = *reinterpret_cast<uint32_t*>(address + 3);
	GlobalObjects = address + 7 + offset;

	printf("GlobalObjects: 0x%x\n", GlobalObjects - (uintptr_t)GetModuleHandleW(0));
	uintptr_t funcGetObjectName;

	const auto address2 = FindPattern(GetModuleHandleW(NULL),
		reinterpret_cast<const unsigned char*>("\x41\xb8\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x48\x8b\xc3\x48\x83\xc4\x00\x5b\xc3\x48\x8b\x42"), "xx????x????xxxxxx?xxxxx");

	funcGetObjectName = address2 - 0x3F;

	printf("funcGetObjectName: 0x%x\n", funcGetObjectName - (uintptr_t)GetModuleHandleW(0));

	/***************-Post render-**********************/
	SDK::UWorld* World = *ue->GWorld;
	auto GameInstance = World->OwningGameInstance;
	auto LocalPlayer = GameInstance->LocalPlayers[0];
	auto ViewportClient = LocalPlayer->ViewportClient;
	// swapping vtables
	auto vTable = *(void***)(ViewportClient);
	origin_renderer = (PostRender_t)vTable[100];
	SwapVtable((void*)ViewportClient, 100, MainRender);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == 1) {
		AllocConsole();
		FILE* fileptr;
		freopen_s(&fileptr, "CONOUT$", "w", stdout);
		freopen_s(&fileptr, "CONOUT$", "w", stderr);
		freopen_s(&fileptr, "CONIN$", "r", stdin);
		main();
	}
	return true;
}
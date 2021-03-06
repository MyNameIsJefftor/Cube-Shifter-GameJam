// Copyright Epic Games, Inc. All Rights Reserved.

#include "CubeShifterHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"

ACubeShifterHUD::ACubeShifterHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/circle-crosshair-png.circle-crosshair-png"/*"/Game/FirstPerson/Textures/FirstPersonCrosshair"*/));
	CrosshairTex = CrosshairTexObj.Object;
}


void ACubeShifterHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - 24.0f),
										   (Center.Y));

	DrawTexture(CrosshairTex, CrosshairDrawPosition.X, CrosshairDrawPosition.Y, 64.0f, 64.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// draw the crosshair
	//FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	//TileItem.BlendMode = SE_BLEND_Translucent;
	//Canvas->DrawItem( TileItem );
}

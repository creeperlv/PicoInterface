#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#define MODE_FAST 0
#define MODE_FLEX 1
float Intensity(Color C){
	return ((float)(C.r+C.g+C.b))/(255.0f*3.0f);
}
int main(int ac,char** av){
	char* fontFile=NULL;
	char* outputFile=NULL;
	for(int i=1;i<ac;i++){
		if(TextIsEqual(av[i],"--i")){
			i++;
			fontFile=av[i];
		}else
		if(TextIsEqual(av[i],"--o")){
			i++;
			outputFile=av[i];
		}
	}
	if(outputFile==NULL){
		outputFile="out.h";
	}
	if(fontFile==NULL){
		fontFile="source.ttf";
	}
	int Mode=MODE_FLEX;
	int L=0;
	int H=0xFF;
	float Threshold=0.3f;
	int FontSize=16;
	int ScanW=10;
	int ScanH=16;
	int ScanOffset=-1;
	int* Codes=malloc(sizeof(int)*(H-L));
	for(int i=L;i<H;i++){
		Codes[i-L]=i;
	}
	FILE* file=fopen(outputFile,"w+");
	if(file==NULL){
		printf("Create a new file!\n");
	}else{
		printf("Replace a file!\n");
		remove(outputFile);
		file=fopen(outputFile,"w+");
	}
	fprintf(file,"#define FONT_HEIGHT %d\n",ScanH);
	fprintf(file,"#define FONT_WIDTH %d\n",ScanW);
	fprintf(file,"#define FONT_MODE %d\n",Mode);
	fprintf(file,"static uint8_t font[] = {\n");
	
	InitWindow(100,100,"Font Gen");
//	SetTargetFPS(1);
	RenderTexture2D target=LoadRenderTexture(FontSize,FontSize);
	RenderTexture2D target2=LoadRenderTexture(FontSize,FontSize);
	SetTextureFilter(target.texture,0);
	SetTextureFilter(target2.texture,0);
	int C=L;
	Image img;
	Font f=LoadFontEx(fontFile,FontSize,Codes,H-L);
	SetTextureFilter(f.texture,0);
	while(!WindowShouldClose()){
		{
			BeginTextureMode(target);
			ClearBackground(BLACK);
			DrawTextCodepoint(f,C,(Vector2){0,0},FontSize,WHITE);
			EndTextureMode();
		}
		{
			BeginTextureMode(target2);
			ClearBackground(BLACK);
			DrawTexturePro(target.texture,(Rectangle){0,0,FontSize,FontSize},(Rectangle){0,0,FontSize,FontSize},(Vector2){0,0},0,WHITE);
			EndTextureMode();
		}
		{
			img=LoadImageFromTexture(target2.texture);
			for(int x=0;x<ScanW;x++){
				unsigned char Pixel=0;
				switch(Mode){
					case MODE_FAST:
						for(int y=0;y<ScanH;y++){
							if(x+ScanOffset>0){
							
								Color c=GetImageColor(img,x+ScanOffset,y);
								float V=Intensity(c);
								if(V>Threshold){
							        Pixel |= 1 << (y % 8);
								}
							}
						}
						printf("\tVL=%u\n",Pixel);
						fprintf(file,"%u,",Pixel);
						
					break;
					case MODE_FLEX:
						for(int y=0;y<ScanH;y++){
							if(x+ScanOffset>0){
							
								Color c=GetImageColor(img,x+ScanOffset,y);
								float V=Intensity(c);
								if(V>Threshold){
									fprintf(file,"1,");
								}else
									fprintf(file,"0,");
							}else{
									fprintf(file,"0,");
							}
						}
						
					break;
				}
			}
				fprintf(file,"\n");
			UnloadImage(img);
		}
		{
			BeginDrawing();
			ClearBackground(BLACK);
			DrawTexturePro(target.texture,(Rectangle){0,0,FontSize,-FontSize},(Rectangle){0,0,100,100},(Vector2){0,0},0,WHITE);
			EndDrawing();
		}
		{
			C++;
			if(C>=H){
			break;
			}
		}
		printf("draw:%c\n",C);
	}
				fprintf(file,"};");
	fflush(file);
	fclose(file);
	return 0;
}

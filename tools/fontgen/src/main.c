#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#define MODE_FAST 0
#define MODE_FLEX 1
float Intensity(Color C) {
  return ((float)(C.r + C.g + C.b)) / (255.0f * 3.0f);
}
int main(int ac, char **av) {
  char *fontFile = NULL;
  char *outputFile = NULL;
  int H = 0xFF;
  int Mode = MODE_FLEX;
  float Threshold = 0.3f;
  int FontSize = 16;
  int ScanW = 10;
  int ScanH = 16;
  int ScanOffset = -1;
  for (int i = 1; i < ac; i++) {
    if (TextIsEqual(av[i], "--i")) {
      i++;
      fontFile = av[i];
    } else if (TextIsEqual(av[i], "--o")) {
      i++;
      outputFile = av[i];
    } else if (TextIsEqual(av[i], "--help") || TextIsEqual(av[i], "-h")) {
      i++;
      printf("Options:\n");
      printf("--i <file>\tInput font file.\n");
      printf("--o <file>\tOutput header file.\n");
      printf("-H,--high <integer>\tHigh character.\n");
      printf("-FS,--font-size <integer>\tFont Size.\n");
      printf("-SH,--scan-height <integer>\tRender Height.\n");
      printf("-M,--mode <integer>\tTranslate mode.\n");
      printf("\t\t%d - Fast mode. Only support 8px in scan height.\n",
             MODE_FAST);
      printf("\t\t%d - Flexible mode. Huge file, slow.\n", MODE_FLEX);
      return 0;
    } else if (TextIsEqual(av[i], "-H") || TextIsEqual(av[i], "--high")) {
      i++;
      H = atoi(av[i]);
    } else if (TextIsEqual(av[i], "-FS") || TextIsEqual(av[i], "--font-size")) {
      i++;
      FontSize = atoi(av[i]);
    } else if (TextIsEqual(av[i], "-SH") ||
               TextIsEqual(av[i], "--scan-height")) {
      i++;
      ScanH = atoi(av[i]);
    } else if (TextIsEqual(av[i], "-M") || TextIsEqual(av[i], "--mode")) {
      i++;
      Mode = atoi(av[i]);
    }
  }
  if (outputFile == NULL) {
    outputFile = "out.h";
  }
  if (fontFile == NULL) {
    fontFile = "source.ttf";
  }
  int L = 0;
  int *Codes = malloc(sizeof(int) * (H - L));
  for (int i = L; i < H; i++) {
    Codes[i - L] = i;
  }
  {
    printf("Render Target:\n");
    printf("High=%d\n", H);
    printf("Font Size=%d\n", FontSize);
    printf("Scan Height=%d\n", ScanH);
    printf("Mode=%d\n", Mode);
    printf("Input=%s\n", fontFile);
    printf("Output=%s\n", outputFile);
  }
  {
    FILE *fontf = fopen(fontFile, "rb");
    if (fontf == NULL) {
      printf("\e[91mFont file %s not found!\e[0m\n", fontFile);
      return -1;
    }
  }
  InitWindow(100, 100, "Font Gen");
  FILE *file = fopen(outputFile, "w+");
  if (file == NULL) {
    printf("Create a new file!\n");
  } else {
    printf("Replace a file!\n");
    remove(outputFile);
    file = fopen(outputFile, "w+");
  }
  fprintf(file, "#define FONT_HEIGHT %d\n", ScanH);
  fprintf(file, "#define FONT_WIDTH %d\n", ScanW);
  fprintf(file, "#define FONT_MODE %d\n", Mode);
  fprintf(file, "static uint8_t font[] = {\n");

  //	SetTargetFPS(1);
  RenderTexture2D target = LoadRenderTexture(FontSize, FontSize);
  RenderTexture2D target2 = LoadRenderTexture(FontSize, FontSize);
  SetTextureFilter(target.texture, 0);
  SetTextureFilter(target2.texture, 0);
  int C = L;
  Image img;
  Font f = LoadFontEx(fontFile, FontSize, Codes, H - L);
  SetTextureFilter(f.texture, 0);
  while (!WindowShouldClose()) {
    {
      BeginTextureMode(target);
      ClearBackground(BLACK);
      DrawTextCodepoint(f, C, (Vector2){0, 0}, FontSize, WHITE);
      EndTextureMode();
    }
    {
      BeginTextureMode(target2);
      ClearBackground(BLACK);
      DrawTexturePro(target.texture, (Rectangle){0, 0, FontSize, FontSize},
                     (Rectangle){0, 0, FontSize, FontSize}, (Vector2){0, 0}, 0,
                     WHITE);
      EndTextureMode();
    }
    {
      img = LoadImageFromTexture(target2.texture);
      for (int x = 0; x < ScanW; x++) {
        unsigned char Pixel = 0;
        switch (Mode) {
        case MODE_FAST:
          for (int y = 0; y < ScanH; y++) {
            if (x + ScanOffset > 0) {

              Color c = GetImageColor(img, x + ScanOffset, y);
              float V = Intensity(c);
              if (V > Threshold) {
                Pixel |= 1 << (y % 8);
              }
            }
          }
          printf("\tVL=%u\n", Pixel);
          fprintf(file, "%u,", Pixel);

          break;
        case MODE_FLEX:
          for (int y = 0; y < ScanH; y++) {
            if (x + ScanOffset > 0) {

              Color c = GetImageColor(img, x + ScanOffset, y);
              float V = Intensity(c);
              if (V > Threshold) {
                fprintf(file, "1,");
              } else
                fprintf(file, "0,");
            } else {
              fprintf(file, "0,");
            }
          }

          break;
        }
      }
      fprintf(file, "\n");
      UnloadImage(img);
    }
    {
      BeginDrawing();
      ClearBackground(BLACK);
      DrawTexturePro(target.texture, (Rectangle){0, 0, FontSize, -FontSize},
                     (Rectangle){0, 0, 100, 100}, (Vector2){0, 0}, 0, WHITE);
      EndDrawing();
    }
    {
      C++;
      if (C >= H) {
        break;
      }
    }
    printf("draw:%c\n", C);
  }
  fprintf(file, "};");
  fflush(file);
  fclose(file);
  return 0;
}

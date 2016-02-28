#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer cbViewPort
// {
//
//   float g_viewPortWidth;             // Offset:    0 Size:     4
//   float g_viewPortHeigh;             // Offset:    4 Size:     4
//   float align1;                      // Offset:    8 Size:     4 [unused]
//   float align2;                      // Offset:   12 Size:     4 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// cbViewPort                        cbuffer      NA          NA    0        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyzw        0     NONE   float   xyz 
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
//
// Constant buffer to DX9 shader constant mappings:
//
// Target Reg Buffer  Start Reg # of Regs        Data Conversion
// ---------- ------- --------- --------- ----------------------
// c1         cb0             0         1  ( FLT, FLT, FLT, FLT)
//
//
// Runtime generated constant mappings:
//
// Target Reg                               Constant Description
// ---------- --------------------------------------------------
// c0                              Vertex Shader position offset
//
//
// Level9 shader bytecode:
//
    vs_2_0
    def c2, 0.5, -1, 1, 0
    dcl_texcoord v0
    dcl_texcoord1 v1
    mov r0.x, c2.x
    mul r0.xy, r0.x, c1
    rcp r0.x, r0.x
    rcp r0.y, r0.y
    mad r1.y, v0.y, -r0.y, c0.y
    mad r1.x, v0.x, r0.x, c0.x
    add oPos.xy, r1, c2.yzzw
    mad oPos.zw, v0.z, c2, c2.xywz
    mov oT0, v1

// approximately 9 instruction slots used
vs_4_0
dcl_constantbuffer cb0[1], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyzw
dcl_output_siv o0.xyzw, position
dcl_output o1.xyzw
dcl_temps 1
mul r0.xy, cb0[0].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
div r0.xy, v0.xyxx, r0.xyxx
mad o0.xy, r0.xyxx, l(1.000000, -1.000000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
mov o0.z, v0.z
mov o0.w, l(1.000000)
mov o1.xyzw, v1.xyzw
ret 
// Approximately 7 instruction slots used
#endif

const BYTE DefaultVertexShaderCode[] =
{
     68,  88,  66,  67, 199, 141, 
    101,  44,  22, 144,  45,  63, 
    160, 130,   5, 229, 154, 203, 
    124, 167,   1,   0,   0,   0, 
    176,   4,   0,   0,   6,   0, 
      0,   0,  56,   0,   0,   0, 
     56,   1,   0,   0,  88,   2, 
      0,   0, 212,   2,   0,   0, 
     12,   4,   0,   0,  92,   4, 
      0,   0,  65, 111, 110,  57, 
    248,   0,   0,   0, 248,   0, 
      0,   0,   0,   2, 254, 255, 
    196,   0,   0,   0,  52,   0, 
      0,   0,   1,   0,  36,   0, 
      0,   0,  48,   0,   0,   0, 
     48,   0,   0,   0,  36,   0, 
      1,   0,  48,   0,   0,   0, 
      0,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   2, 254, 255, 
     81,   0,   0,   5,   2,   0, 
     15, 160,   0,   0,   0,  63, 
      0,   0, 128, 191,   0,   0, 
    128,  63,   0,   0,   0,   0, 
     31,   0,   0,   2,   5,   0, 
      0, 128,   0,   0,  15, 144, 
     31,   0,   0,   2,   5,   0, 
      1, 128,   1,   0,  15, 144, 
      1,   0,   0,   2,   0,   0, 
      1, 128,   2,   0,   0, 160, 
      5,   0,   0,   3,   0,   0, 
      3, 128,   0,   0,   0, 128, 
      1,   0, 228, 160,   6,   0, 
      0,   2,   0,   0,   1, 128, 
      0,   0,   0, 128,   6,   0, 
      0,   2,   0,   0,   2, 128, 
      0,   0,  85, 128,   4,   0, 
      0,   4,   1,   0,   2, 128, 
      0,   0,  85, 144,   0,   0, 
     85, 129,   0,   0,  85, 160, 
      4,   0,   0,   4,   1,   0, 
      1, 128,   0,   0,   0, 144, 
      0,   0,   0, 128,   0,   0, 
      0, 160,   2,   0,   0,   3, 
      0,   0,   3, 192,   1,   0, 
    228, 128,   2,   0, 233, 160, 
      4,   0,   0,   4,   0,   0, 
     12, 192,   0,   0, 170, 144, 
      2,   0, 228, 160,   2,   0, 
    180, 160,   1,   0,   0,   2, 
      0,   0,  15, 224,   1,   0, 
    228, 144, 255, 255,   0,   0, 
     83,  72,  68,  82,  24,   1, 
      0,   0,  64,   0,   1,   0, 
     70,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   0,   0, 
      0,   0,  95,   0,   0,   3, 
    242,  16,  16,   0,   1,   0, 
      0,   0, 103,   0,   0,   4, 
    242,  32,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 242,  32, 
     16,   0,   1,   0,   0,   0, 
    104,   0,   0,   2,   1,   0, 
      0,   0,  56,   0,   0,  11, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70, 128,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,  63,   0,   0, 
      0,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,  14,   0, 
      0,   7,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,  16, 
     16,   0,   0,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0,  50,   0,   0,  15, 
     50,  32,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0, 128,  63, 
      0,   0, 128, 191,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128, 191,   0,   0, 128,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     66,  32,  16,   0,   0,   0, 
      0,   0,  42,  16,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,  32,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     54,   0,   0,   5, 242,  32, 
     16,   0,   1,   0,   0,   0, 
     70,  30,  16,   0,   1,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 116,   0, 
      0,   0,   7,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  82,  68, 
     69,  70,  48,   1,   0,   0, 
      1,   0,   0,   0,  72,   0, 
      0,   0,   1,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    254, 255,   0,   1,   0,   0, 
    254,   0,   0,   0,  60,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
     99,  98,  86, 105, 101, 119, 
     80, 111, 114, 116,   0, 171, 
     60,   0,   0,   0,   4,   0, 
      0,   0,  96,   0,   0,   0, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    192,   0,   0,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0, 208,   0, 
      0,   0,   0,   0,   0,   0, 
    224,   0,   0,   0,   4,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0, 208,   0, 
      0,   0,   0,   0,   0,   0, 
    240,   0,   0,   0,   8,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0, 208,   0, 
      0,   0,   0,   0,   0,   0, 
    247,   0,   0,   0,  12,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0, 208,   0, 
      0,   0,   0,   0,   0,   0, 
    103,  95, 118, 105, 101, 119, 
     80, 111, 114, 116,  87, 105, 
    100, 116, 104,   0,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 103,  95, 118, 105, 
    101, 119,  80, 111, 114, 116, 
     72, 101, 105, 103, 104,   0, 
     97, 108, 105, 103, 110,  49, 
      0,  97, 108, 105, 103, 110, 
     50,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  54,  46,  51,  46,  57, 
     54,  48,  48,  46,  49,  54, 
     51,  56,  52,   0,  73,  83, 
     71,  78,  72,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   7, 
      0,   0,  65,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  15,  15, 
      0,   0,  80,  79,  83,  73, 
     84,  73,  79,  78,   0,  67, 
     79,  76,  79,  82,   0, 171, 
     79,  83,  71,  78,  76,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,  68,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  80,  79,  83,  73,  84, 
     73,  79,  78,   0,  67,  79, 
     76,  79,  82,   0, 171, 171
};
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

int globalArray[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

void *switchGet(Key key) {
  switch(key) {
    case 1: return &globalArray[7]; break;
    case 2: return &globalArray[14]; break;
    case 3: return &globalArray[21]; break;
    case 4: return &globalArray[28]; break;
    case 5: return &globalArray[35]; break;
    case 6: return &globalArray[42]; break;
    case 7: return &globalArray[49]; break;
    case 8: return &globalArray[56]; break;
    case 9: return &globalArray[63]; break;
    case 10: return &globalArray[70]; break;
    case 11: return &globalArray[77]; break;
    case 12: return &globalArray[84]; break;
    case 13: return &globalArray[91]; break;
    case 14: return &globalArray[98]; break;
    case 15: return &globalArray[105]; break;
    case 16: return &globalArray[112]; break;
    case 17: return &globalArray[119]; break;
    case 18: return &globalArray[126]; break;
    case 19: return &globalArray[133]; break;
    case 20: return &globalArray[140]; break;
    case 21: return &globalArray[147]; break;
    case 22: return &globalArray[154]; break;
    case 23: return &globalArray[161]; break;
    case 24: return &globalArray[168]; break;
    case 25: return &globalArray[175]; break;
    case 26: return &globalArray[182]; break;
    case 27: return &globalArray[189]; break;
    case 28: return &globalArray[196]; break;
    case 29: return &globalArray[203]; break;
    case 30: return &globalArray[210]; break;
    case 31: return &globalArray[217]; break;
    case 32: return &globalArray[224]; break;
    case 33: return &globalArray[231]; break;
    case 34: return &globalArray[238]; break;
    case 35: return &globalArray[245]; break;
    case 36: return &globalArray[252]; break;
    case 37: return &globalArray[4]; break;
    case 38: return &globalArray[11]; break;
    case 39: return &globalArray[18]; break;
    case 40: return &globalArray[25]; break;
    case 41: return &globalArray[32]; break;
    case 42: return &globalArray[39]; break;
    case 43: return &globalArray[46]; break;
    case 44: return &globalArray[53]; break;
    case 45: return &globalArray[60]; break;
    case 46: return &globalArray[67]; break;
    case 47: return &globalArray[74]; break;
    case 48: return &globalArray[81]; break;
    case 49: return &globalArray[88]; break;
    case 50: return &globalArray[95]; break;
    case 51: return &globalArray[102]; break;
    case 52: return &globalArray[109]; break;
    case 53: return &globalArray[116]; break;
    case 54: return &globalArray[123]; break;
    case 55: return &globalArray[130]; break;
    case 56: return &globalArray[137]; break;
    case 57: return &globalArray[144]; break;
    case 58: return &globalArray[151]; break;
    case 59: return &globalArray[158]; break;
    case 60: return &globalArray[165]; break;
    case 61: return &globalArray[172]; break;
    case 62: return &globalArray[179]; break;
    case 63: return &globalArray[186]; break;
    case 64: return &globalArray[193]; break;
    case 65: return &globalArray[200]; break;
    case 66: return &globalArray[207]; break;
    case 67: return &globalArray[214]; break;
    case 68: return &globalArray[221]; break;
    case 69: return &globalArray[228]; break;
    case 70: return &globalArray[235]; break;
    case 71: return &globalArray[242]; break;
    case 72: return &globalArray[249]; break;
    case 73: return &globalArray[1]; break;
    case 74: return &globalArray[8]; break;
    case 75: return &globalArray[15]; break;
    case 76: return &globalArray[22]; break;
    case 77: return &globalArray[29]; break;
    case 78: return &globalArray[36]; break;
    case 79: return &globalArray[43]; break;
    case 80: return &globalArray[50]; break;
    case 81: return &globalArray[57]; break;
    case 82: return &globalArray[64]; break;
    case 83: return &globalArray[71]; break;
    case 84: return &globalArray[78]; break;
    case 85: return &globalArray[85]; break;
    case 86: return &globalArray[92]; break;
    case 87: return &globalArray[99]; break;
    case 88: return &globalArray[106]; break;
    case 89: return &globalArray[113]; break;
    case 90: return &globalArray[120]; break;
    case 91: return &globalArray[127]; break;
    case 92: return &globalArray[134]; break;
    case 93: return &globalArray[141]; break;
    case 94: return &globalArray[148]; break;
    case 95: return &globalArray[155]; break;
    case 96: return &globalArray[162]; break;
    case 97: return &globalArray[169]; break;
    case 98: return &globalArray[176]; break;
    case 99: return &globalArray[183]; break;
    case 100: return &globalArray[190]; break;
    case 101: return &globalArray[197]; break;
    case 102: return &globalArray[204]; break;
    case 103: return &globalArray[211]; break;
    case 104: return &globalArray[218]; break;
    case 105: return &globalArray[225]; break;
    case 106: return &globalArray[232]; break;
    case 107: return &globalArray[239]; break;
    case 108: return &globalArray[246]; break;
    case 109: return &globalArray[253]; break;
    case 110: return &globalArray[5]; break;
    case 111: return &globalArray[12]; break;
    case 112: return &globalArray[19]; break;
    case 113: return &globalArray[26]; break;
    case 114: return &globalArray[33]; break;
    case 115: return &globalArray[40]; break;
    case 116: return &globalArray[47]; break;
    case 117: return &globalArray[54]; break;
    case 118: return &globalArray[61]; break;
    case 119: return &globalArray[68]; break;
    case 120: return &globalArray[75]; break;
    case 121: return &globalArray[82]; break;
    case 122: return &globalArray[89]; break;
    case 123: return &globalArray[96]; break;
    case 124: return &globalArray[103]; break;
    case 125: return &globalArray[110]; break;
    case 126: return &globalArray[117]; break;
    case 127: return &globalArray[124]; break;
    case 128: return &globalArray[131]; break;
    case 129: return &globalArray[138]; break;
    case 130: return &globalArray[145]; break;
    case 131: return &globalArray[152]; break;
    case 132: return &globalArray[159]; break;
    case 133: return &globalArray[166]; break;
    case 134: return &globalArray[173]; break;
    case 135: return &globalArray[180]; break;
    case 136: return &globalArray[187]; break;
    case 137: return &globalArray[194]; break;
    case 138: return &globalArray[201]; break;
    case 139: return &globalArray[208]; break;
    case 140: return &globalArray[215]; break;
    case 141: return &globalArray[222]; break;
    case 142: return &globalArray[229]; break;
    case 143: return &globalArray[236]; break;
    case 144: return &globalArray[243]; break;
    case 145: return &globalArray[250]; break;
    case 146: return &globalArray[2]; break;
    case 147: return &globalArray[9]; break;
    case 148: return &globalArray[16]; break;
    case 149: return &globalArray[23]; break;
    case 150: return &globalArray[30]; break;
    case 151: return &globalArray[37]; break;
    case 152: return &globalArray[44]; break;
    case 153: return &globalArray[51]; break;
    case 154: return &globalArray[58]; break;
    case 155: return &globalArray[65]; break;
    case 156: return &globalArray[72]; break;
    case 157: return &globalArray[79]; break;
    case 158: return &globalArray[86]; break;
    case 159: return &globalArray[93]; break;
    case 160: return &globalArray[100]; break;
    case 161: return &globalArray[107]; break;
    case 162: return &globalArray[114]; break;
    case 163: return &globalArray[121]; break;
    case 164: return &globalArray[128]; break;
    case 165: return &globalArray[135]; break;
    case 166: return &globalArray[142]; break;
    case 167: return &globalArray[149]; break;
    case 168: return &globalArray[156]; break;
    case 169: return &globalArray[163]; break;
    case 170: return &globalArray[170]; break;
    case 171: return &globalArray[177]; break;
    case 172: return &globalArray[184]; break;
    case 173: return &globalArray[191]; break;
    case 174: return &globalArray[198]; break;
    case 175: return &globalArray[205]; break;
    case 176: return &globalArray[212]; break;
    case 177: return &globalArray[219]; break;
    case 178: return &globalArray[226]; break;
    case 179: return &globalArray[233]; break;
    case 180: return &globalArray[240]; break;
    case 181: return &globalArray[247]; break;
    case 182: return &globalArray[254]; break;
    case 183: return &globalArray[6]; break;
    case 184: return &globalArray[13]; break;
    case 185: return &globalArray[20]; break;
    case 186: return &globalArray[27]; break;
    case 187: return &globalArray[34]; break;
    case 188: return &globalArray[41]; break;
    case 189: return &globalArray[48]; break;
    case 190: return &globalArray[55]; break;
    case 191: return &globalArray[62]; break;
    case 192: return &globalArray[69]; break;
    case 193: return &globalArray[76]; break;
    case 194: return &globalArray[83]; break;
    case 195: return &globalArray[90]; break;
    case 196: return &globalArray[97]; break;
    case 197: return &globalArray[104]; break;
    case 198: return &globalArray[111]; break;
    case 199: return &globalArray[118]; break;
    case 200: return &globalArray[125]; break;
    case 201: return &globalArray[132]; break;
    case 202: return &globalArray[139]; break;
    case 203: return &globalArray[146]; break;
    case 204: return &globalArray[153]; break;
    case 205: return &globalArray[160]; break;
    case 206: return &globalArray[167]; break;
    case 207: return &globalArray[174]; break;
    case 208: return &globalArray[181]; break;
    case 209: return &globalArray[188]; break;
    case 210: return &globalArray[195]; break;
    case 211: return &globalArray[202]; break;
    case 212: return &globalArray[209]; break;
    case 213: return &globalArray[216]; break;
    case 214: return &globalArray[223]; break;
    case 215: return &globalArray[230]; break;
    case 216: return &globalArray[237]; break;
    case 217: return &globalArray[244]; break;
    case 218: return &globalArray[251]; break;
    case 219: return &globalArray[3]; break;
    case 220: return &globalArray[10]; break;
    case 221: return &globalArray[17]; break;
    case 222: return &globalArray[24]; break;
    case 223: return &globalArray[31]; break;
    case 224: return &globalArray[38]; break;
    case 225: return &globalArray[45]; break;
    case 226: return &globalArray[52]; break;
    case 227: return &globalArray[59]; break;
    case 228: return &globalArray[66]; break;
    case 229: return &globalArray[73]; break;
    case 230: return &globalArray[80]; break;
    case 231: return &globalArray[87]; break;
    case 232: return &globalArray[94]; break;
    case 233: return &globalArray[101]; break;
    case 234: return &globalArray[108]; break;
    case 235: return &globalArray[115]; break;
    case 236: return &globalArray[122]; break;
    case 237: return &globalArray[129]; break;
    case 238: return &globalArray[136]; break;
    case 239: return &globalArray[143]; break;
    case 240: return &globalArray[150]; break;
    case 241: return &globalArray[157]; break;
    case 242: return &globalArray[164]; break;
    case 243: return &globalArray[171]; break;
    case 244: return &globalArray[178]; break;
    case 245: return &globalArray[185]; break;
    case 246: return &globalArray[192]; break;
    case 247: return &globalArray[199]; break;
    case 248: return &globalArray[206]; break;
    case 249: return &globalArray[213]; break;
    case 250: return &globalArray[220]; break;
    case 251: return &globalArray[227]; break;
    case 252: return &globalArray[234]; break;
    case 253: return &globalArray[241]; break;
    case 254: return &globalArray[248]; break;
    case 255: return &globalArray[0]; break;
  }
  return (void*) &globalArray[0];
}

int main(int argc, char **argv) {
  Map *mapP;
  Error e = mapNew(&mapP, RAW_DATA, sizeof(int), 255);

  for (int i = 1; !e && i <= 255; i++) {
    e = mapSet(mapP, i, (void*) &i);
  }

  for (int i = 1; i < 256; i++) {
    //int* iP = (int*) mapGet(mapP, (i & 0xFF) | 1);
    int* iP = (int*) mapGet(mapP, i);
    printf("%d: %d\n", i, *iP);
  }
#if 0
  for (int i = 1; i < 1000000; i++) {
    int *iP = switchGet((i & 0xFF) | 1);
    printf("switchGet got %d\n", *iP);
  }
#endif
  mapDel(&mapP);
  return 0;
}

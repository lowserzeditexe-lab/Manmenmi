# Graphics IR — M6 partial shader IR

M2/M3 : intentions de transfert et de rendu minimales.
M6 : une IR shader privée et structurée couvre un micro-sous-ensemble du
triangle (inputs position/couleur, constantes/addition float4, construction de
position, move et outputs). Elle est traduite vers HLSL puis compilée par
DX12. Aucun opcode Latte, blob GX2 réel, table de swizzle ou parser complet
n'est implémenté.
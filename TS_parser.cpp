#include "tsCommon.h"
#include "tsTransportStream.h"

//=============================================================================================================================================================================

int main(int argc, char *argv[ ], char *envp[ ])
{

  FILE *exampleFile = fopen("example_new.ts", "rb");

  if ( exampleFile ){
    printf("Odpalil! \n");
  } else {
    printf("Nie odpalil :c \n");
  }

  xTS_PacketHeader TS_PacketHeader;
  xTS_AdaptationField TS_AdaptationField;
  xPES_Assembler PES_Assembler;

  PES_Assembler.Init(136);

  const int packetSize = xTS::TS_PacketLength; // Wskazuje ile bajtow ma pakiet
  uint8_t buffor[packetSize]; // Tworzy bufor na pojedynczy pakiet

  int32_t TS_PacketId = 0;
  while( !feof(exampleFile) )
  {
    size_t readed = fread( buffor, sizeof( uint8_t ), packetSize, exampleFile );

    TS_PacketHeader.Reset();
    TS_PacketHeader.Parse(buffor);

    TS_AdaptationField.Reset();
    if(TS_PacketHeader.getSyncByte() == 'G' && TS_PacketHeader.getPacketIdentifier() == 136){
      if(TS_PacketHeader.hasAdaptationField()){
        TS_AdaptationField.Parse(buffor, TS_PacketHeader.getAdaptationFieldControl());
      }

      printf(/*KRED*/ "%010d ", TS_PacketId);
      TS_PacketHeader.Print();
      if(TS_PacketHeader.hasAdaptationField()){ TS_AdaptationField.Print();}

      xPES_Assembler::eResult Result = PES_Assembler.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
      switch(Result){
        case xPES_Assembler::eResult::StreamPacketLost : printf("Packet Lost."); break;
        case xPES_Assembler::eResult::AssemblingStarted : printf(/*KWHT*/ "Start "); PES_Assembler.PrintPESH(); break;
        case xPES_Assembler::eResult::AssemblingFinished : printf(/*KWHT*/ "Finish "); printf(/*KBLU*/ "\n\t   PES: Packet Length: %d, Header Length: %d, Data Length: %d", PES_Assembler.getNumPacketBytes(), PES_Assembler.getHeaderLength(), (PES_Assembler.getNumPacketBytes()-PES_Assembler.getHeaderLength())); break;
        case xPES_Assembler::eResult::AssemblingContinue : printf("Continue"); break;
        default : break;
      }

      printf("\n");
    }

    // if(TS_PacketId == 34){
    //     break;
    // }

    TS_PacketId++;
  }

  fclose(exampleFile);

  return EXIT_SUCCESS;
}

//=============================================================================================================================================================================

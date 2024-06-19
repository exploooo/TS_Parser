#include "tsCommon.h"
#include "tsTransportStream.h"

//=============================================================================================================================================================================

int main(int argc, char *argv[ ], char *envp[ ])
{

  /* 
     Determine which System is used whilst checking
     Purely for me at the moment, to make testing easier
     TODO: change to os library to check for uname,
     and boot optimal GUI system for the OS.
  */
  char osType[] = "Linux";
  char path[128];

  if(!strcmp(osType,"Linux")){
    strcpy(path,"/home/explo/Documents/src/TS_Parser/example_new.ts");
  } else if(!strcmp(osType,"Windows")){
    strcpy(path,"example_new.ts");
  }

  /* 
     Switches that makes life easier
     Those are ment to be set in this section of code
  */

  bool wavOutputRequired = true; // Is .wav output required? Requires mpeg123 libs
  bool yetAnotherVerboseModeSwitch = false; // Do You Want to show packet messages?
  uint32_t breakAt = 0; // if You wish You can end at any packet

  /*
    Main Program
  */

  FILE *exampleFile = fopen(path, "rb");

  if ( !exampleFile ){
    printf("Nie odpalil :c\n");
    return -1;
  }

  printf("Odpalil!\n");

  xTS_PacketHeader TS_PacketHeader;
  xTS_AdaptationField TS_AdaptationField;
  xPES_Assembler PES_Assembler_Audio, PES_Assembler_Video;

  PES_Assembler_Audio.Init(136);
  PES_Assembler_Video.Init(174);

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

      xPES_Assembler::eResult Result = PES_Assembler_Audio.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);

      if(yetAnotherVerboseModeSwitch){

        printf(KMAG "%010d ", TS_PacketId);
        TS_PacketHeader.Print();
        if(TS_PacketHeader.hasAdaptationField()){ TS_AdaptationField.Print();}

        switch(Result){
          case xPES_Assembler::eResult::StreamPacketLost : printf("Packet Lost."); break;
          case xPES_Assembler::eResult::AssemblingStarted : printf(KCYN "Start "); PES_Assembler_Audio.PrintPESH(); break;
          case xPES_Assembler::eResult::AssemblingFinished :
            printf(KCYN "Finish "); 
            printf(KBLU "\n\t   PES: Packet Length: %d, Header Length: %d, Data Length: %d", PES_Assembler_Audio.getNumPacketBytes(), PES_Assembler_Audio.getHeaderLength(), (PES_Assembler_Audio.getNumPacketBytes()-PES_Assembler_Audio.getHeaderLength()));
            break;
          case xPES_Assembler::eResult::AssemblingFinishedToStart: 
            printf(KCYN "Finish ");
            printf(KBLU "\n\t   PES: Packet Length: %d, Header Length: %d, Data Length: %d", PES_Assembler_Audio.getNumPacketBytes(), PES_Assembler_Audio.getHeaderLength(), (PES_Assembler_Audio.getNumPacketBytes()-PES_Assembler_Audio.getHeaderLength()));
            if(TS_PacketHeader.getPacketIdentifier() == 136){
              Result = PES_Assembler_Audio.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
            } else if(TS_PacketHeader.getPacketIdentifier() == 174){
              Result = PES_Assembler_Video.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
            }
            printf(KCYN "\nStart ");
            PES_Assembler_Video.PrintPESH();
            break;
          case xPES_Assembler::eResult::AssemblingContinue : printf(KCYN "Continue"); break;
          default : break;
        }

        printf("\n");
      }

      if(!yetAnotherVerboseModeSwitch && Result == xPES_Assembler::eResult::AssemblingFinishedToStart){
          if(TS_PacketHeader.getPacketIdentifier() == 136){
            Result = PES_Assembler_Audio.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
          } else if(TS_PacketHeader.getPacketIdentifier() == 174){
            Result = PES_Assembler_Video.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
          }
      }


    } else if(TS_PacketHeader.getSyncByte() == 'G' && TS_PacketHeader.getPacketIdentifier() == 174){
        if(TS_PacketHeader.hasAdaptationField()){
          TS_AdaptationField.Parse(buffor, TS_PacketHeader.getAdaptationFieldControl());
        }

        xPES_Assembler::eResult Result = PES_Assembler_Video.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);

        if(yetAnotherVerboseModeSwitch){

          printf(KMAG "%010d ", TS_PacketId);
          TS_PacketHeader.Print();
          if(TS_PacketHeader.hasAdaptationField()){ TS_AdaptationField.Print();}
          
          switch(Result){
            case xPES_Assembler::eResult::StreamPacketLost : printf("Packet Lost."); break;
            case xPES_Assembler::eResult::AssemblingStarted : printf(KCYN "Start "); PES_Assembler_Video.PrintPESH(); break;
            case xPES_Assembler::eResult::AssemblingFinished :
              printf(KCYN "Finish "); 
              printf(KBLU "\n\t   PES: Packet Length: %d, Header Length: %d, Data Length: %d", PES_Assembler_Video.getNumPacketBytes(), PES_Assembler_Video.getHeaderLength(), (PES_Assembler_Video.getNumPacketBytes()-PES_Assembler_Video.getHeaderLength()));
              break;
            case xPES_Assembler::eResult::AssemblingFinishedToStart: 
              printf(KCYN"Finish ");
              printf(KBLU "\n\t   PES: Packet Length: %d, Header Length: %d, Data Length: %d", PES_Assembler_Video.getNumPacketBytes(), PES_Assembler_Video.getHeaderLength(), (PES_Assembler_Video.getNumPacketBytes()-PES_Assembler_Video.getHeaderLength()));
              if(TS_PacketHeader.getPacketIdentifier() == 136){
                Result = PES_Assembler_Audio.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
              } else if(TS_PacketHeader.getPacketIdentifier() == 174){
                Result = PES_Assembler_Video.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
              }
              printf(KCYN"\nStart ");
              PES_Assembler_Video.PrintPESH();
              break;
            case xPES_Assembler::eResult::AssemblingContinue : printf(KCYN "Continue"); break;
            default : break;
          }

          printf("\n");
        }

        if(!yetAnotherVerboseModeSwitch && Result == xPES_Assembler::eResult::AssemblingFinishedToStart){
          if(TS_PacketHeader.getPacketIdentifier() == 136){
            Result = PES_Assembler_Audio.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
          } else if(TS_PacketHeader.getPacketIdentifier() == 174){
            Result = PES_Assembler_Video.AbsorbPacket(buffor, &TS_PacketHeader, &TS_AdaptationField);
          }
        }

    }

  if(breakAt && TS_PacketId == breakAt){
    break;
  }

    TS_PacketId++;
  }

  printf("Koniec!\n");
  fclose(exampleFile);

  if(wavOutputRequired){

    FILE* mpeg2audioFile = fopen("PID136.mp2", "rb");
    mpg123_handle* mh;
    uint8_t* buffer[8];

    while(!feof(mpeg2audioFile)){
      size_t readed = fread( buffer, sizeof( uint8_t ), 1, mpeg2audioFile ); 
      
    }
    
  }

  return EXIT_SUCCESS;
}

//=============================================================================================================================================================================

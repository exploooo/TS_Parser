#include "tsTransportStream.h"
#include <iostream>
#include <vector>

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================


/// @brief Reset - reset all TS packet header fields
void xTS_PacketHeader::Reset()
{
  m_SB = 0;
  m_E = 0;
  m_S = 0;
  m_T = 0;
  m_PID = 0;
  m_TSC = 0;
  m_AFC = 0;
  m_CC = 0;
}

/**
  @brief Parse all TS packet header fields
  @param Input is pointer to buffer containing TS packet
  @return Number of parsed bytes (4 on success, -1 on failure)
 */
int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{

/* Mozna zobaczyc jak leci ladnie
    printf("Bajt 1: ");
    ReadInputBinary8(*Input); printf("\t");
    printf("Bajt 2: ");
    ReadInputBinary8(*(Input+1)); printf("\t");
    printf("Bajt 3: ");
    ReadInputBinary8(*(Input+2)); printf("\t");
    printf("Bajt 4: ");
    ReadInputBinary8(*(Input+3)); printf("\t");
*/

    uint32_t header = Input[0] | Input[1]<<8 | Input[2]<<16 | Input[3]<<24;
    header = xSwapBytes32(header);

    if(!header){
        return -1;
    }

/*  Tu tez
    printf("\nCaly pakiet: ");
    ReadInputBinary32(header);
*/

  m_SB = (header&0xff000000)>>24;
  m_E = (header&0x800000)>>23;
  m_S = (header&0x400000)>>22;
  m_T = (header&0x200000)>>21;
  m_PID = (header&0x1fff00)>>8;
  m_TSC = (header&0xc0)>>6;
  m_AFC = (header&0x30)>>4;
  m_CC = (header&0xf);

  return 4;
}

/// @brief Print all TS packet header fields
void xTS_PacketHeader::Print() const
{
   printf(KWHT "TS: SB: %u E: %u S: %u T: %u PID: %4u TSC: %u AFC: %u CC: %2u ", getSyncByte(), getTransportErrorIndicator(), getPayloadUntilStartIndicator(), getTransportPriority(), getPacketIdentifier(), getTransportScramblingControl(), getAdaptationFieldControl(), getContinuityCounter());
}

void xTS_PacketHeader::ReadInputBinary8(uint8_t byte){

      for (uint8_t mask=128;mask;){
        if(mask&byte){
            printf("1");
        } else{
            printf("0");
        }

        mask=mask>>1;
    }

    printf("\t");

}

void xTS_PacketHeader::ReadInputBinary32(uint32_t bytes){

      for (uint32_t mask=0x80000000;mask;){
        if(mask&bytes){
            printf("1");
        } else{
            printf("0");
        }

        mask=mask>>1;
    }

    printf("\t");

}

//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_AdaptationField::Reset()
{
    m_AFL = 0;
    m_DC = 0;
    m_RA = 0;
    m_SP = 0;
    m_PR = 0;
    m_OR = 0;
    m_SF = 0;
    m_TP = 0;
    m_EX = 0;
    //optional fields - PCR
    m_PCRB = 0;
    m_PCRE = 0;
    m_PCR = 0;
//optional fields - OPCR
    m_OPCRB = 0;
    m_OPCRE = 0;
    m_OPCR = 0;
//optional field - TP LENGTH
    m_TPLen = 0;
//optional field - AFE LENGTH
    m_AFELen = 0;
//derived
    m_StuffingBytesCount=0;
}
/**
@brief Parse adaptation field
@param PacketBuffer is pointer to buffer containing TS packet
@param AdaptationFieldControl is value of Adaptation Field Control field of
corresponding TS packet header
@return Number of parsed bytes (length of AF or -1 on failure)
*/
int32_t xTS_AdaptationField::Parse(const uint8_t* PacketBuffer, uint8_t AdaptationFieldControl)
{
    if(AdaptationFieldControl != 2 && AdaptationFieldControl != 3){
        return NOT_VALID;
    }

    PacketBuffer+=4;
    //parsing
    m_AFL = *PacketBuffer;
    m_StuffingBytesCount=m_AFL;
    if(m_AFL > 0){
        PacketBuffer+=1; m_StuffingBytesCount--;
        m_DC=((*PacketBuffer)&0b10000000)>>7;
        m_RA=((*PacketBuffer)&0b01000000)>>6;
        m_SP=((*PacketBuffer)&0b00100000)>>5;
        m_PR=((*PacketBuffer)&0b00010000)>>4;
        m_OR=((*PacketBuffer)&0b00001000)>>3;
        m_SF=((*PacketBuffer)&0b00000100)>>2;
        m_TP=((*PacketBuffer)&0b00000010)>>1;
        m_EX=((*PacketBuffer)&0b00000001);
        if(m_PR){
            PacketBuffer+=1;
            m_PCRB = PacketBuffer[0]<<25 | PacketBuffer[1]<<17 | PacketBuffer[2]<<9 | PacketBuffer[3]<<1 | PacketBuffer[5]>>7;
            m_PCRE = (PacketBuffer[5]&0b00000001)<<8 | PacketBuffer[6];
            m_PCR=(m_PCRB*300) + m_PCRE;
            PacketBuffer+=5;
            m_StuffingBytesCount-=6;
        }
        if(m_OR){
            PacketBuffer+=1;
            m_OPCRB = PacketBuffer[0]<<25 | PacketBuffer[1]<<17 | PacketBuffer[2]<<9 | PacketBuffer[3]<<1 | PacketBuffer[5]>>7;
            m_OPCRE = (PacketBuffer[5]&0b00000001)<<8 | PacketBuffer[6];
            m_OPCR=(m_OPCRB*300) + m_OPCRE;
            PacketBuffer+=5;
            m_StuffingBytesCount-=6;
        }
        if(m_SF){
            PacketBuffer+=1;
            m_StuffingBytesCount-=1;
        }
        if(m_TP){
            PacketBuffer+=1;
            m_TPLen = *PacketBuffer;
            m_StuffingBytesCount-=m_TPLen;
        }
        if(m_EX){
            PacketBuffer+=1;
            m_AFELen= *PacketBuffer;
            m_StuffingBytesCount-=m_AFELen;
        }
    }
    return (m_AFL+1);
}
/// @brief Print all TS packet header fields
void xTS_AdaptationField::Print() const
{
//print print print
    printf(KYEL "\n\t   AF: L: %3d DC: %d RA: %d SP: %d PR: %d OR: %d SF: %d TP: %d EX: %d", m_AFL, m_DC, m_RA, m_SP, m_PR, m_OR, m_SF, m_TP, m_EX);
    if(m_PR == true){printf(" PCR: %ld (Time: %.5f sec)", m_PCR, ((float)m_PCR/27000000));}
    if(m_OR == true){printf(" OPCR: %ld (Time: %.5f sec)", m_OPCR, ((float)m_OPCR/27000000));}
    printf(" Stuffing Bytes: %3d ", m_StuffingBytesCount);
}

void xTS_AdaptationField::ReadInputBinary8(uint8_t byte){

    printf("\t");

      for (uint8_t mask=128;mask;){
        if(mask&byte){
            printf("1");
        } else{
            printf("0");
        }

        mask=mask>>1;
    }


}

//=============================================================================================================================================================================

void xPES_PacketHeader::Reset(){
    m_PacketStartCodePrefix = 0;
    m_StreamId = 0;
    m_PacketLength = 0;
    m_PTS_DTS_Flags = 0;
    m_PTS = 0;
    m_DTS = 0;
    m_HeaderLength = 6;
    m_ExtendedHeaderDataLength = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input){

    m_PacketStartCodePrefix = Input[0]<<16 | Input[1]<<8 | Input[2];
    m_StreamId = Input[3];
    m_PacketLength = Input[4]<<8 | Input[5];

    if(m_StreamId!=eStreamId::eStreamId_DSMCC_stream && m_StreamId!=eStreamId::eStreamId_ECM && m_StreamId!=eStreamId::eStreamId_EMM && m_StreamId!=eStreamId::eStreamId_ITUT_H222_1_type_E && m_StreamId!=eStreamId::eStreamId_padding_stream && m_StreamId != eStreamId::eStreamId_private_stream_2 && m_StreamId != eStreamId::eStreamId_program_stream_directory && m_StreamId != eStreamId::eStreamId_program_stream_map){
        m_HeaderLength+=3;
        m_PTS_DTS_Flags = (Input[7]&0b11000000)>>6;
        m_ExtendedHeaderDataLength = Input[8];
        if(m_PTS_DTS_Flags == 2){
            m_PTS = (Input[9]&0b00001110)<<28 | Input[10]<<21 | (Input[11]&0b11111110)<<14 | Input[12]<<7  | (Input[13]&0b11111110)>>1;
            //printf("\n\n"); ReadInputBinary8(Input[9]); ReadInputBinary8(Input[10]); ReadInputBinary8(Input[11]); ReadInputBinary8(Input[12]); ReadInputBinary8(Input[13]); printf("\n\n");
        } else if(m_PTS_DTS_Flags == 3){
            m_PTS = (Input[9]&0b00001110)<<28 | Input[10]<<21 | (Input[11]&0b11111110)<<14 | Input[12]<<7  | (Input[13]&0b11111110)>>1;
            m_DTS = (Input[14]&0b00001110)<<28 | Input[15]<<21 | (Input[16]&0b11111110)<<14 | Input[17]<<7  | (Input[18]&0b11111110)>>1;
        }
    } else{
        m_ExtendedHeaderDataLength = 0;
    }

    m_HeaderLength+=m_ExtendedHeaderDataLength;

    return 1;
}

void xPES_PacketHeader::Print() const
{
    printf(KBLU "\n\t   PESH: PSCP: %d, SID: %d, PL: %d, EPL: %d", m_PacketStartCodePrefix, m_StreamId, m_PacketLength, m_ExtendedHeaderDataLength);
    if(m_PTS_DTS_Flags==2){ printf(", PTS: %d(Time: %.5f sec.)", m_PTS, ((float)m_PTS/90000)); }
    if(m_PTS_DTS_Flags==3){ printf(", PTS: %d(Time: %.5f sec.), DTS: %d(Time: %.5f sec.)", m_PTS, ((float)m_PTS/90000), m_DTS, ((float)m_DTS/90000) ); }

}

void xPES_PacketHeader::ReadInputBinary8(uint8_t byte){

    printf("\t");

      for (uint8_t mask=128;mask;){
        if(mask&byte){
            printf("1");
        } else{
            printf("0");
        }

        mask=mask>>1;
    }


}

//=============================================================================================================================================================================

void xPES_Assembler::Init(int32_t PID){
    m_PID = PID;
    m_Started = false;
    if(m_PID == 136){
        m_streamDataFile = fopen("PID136.mp2", "wb");
    } else if(m_PID == 174){
        m_streamDataFile = fopen("PID174.264", "wb");
    }
}

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField){

    if(PacketHeader->getPacketIdentifier() != m_PID){
        m_LastContinuityCounter=PacketHeader->getContinuityCounter();
        return xPES_Assembler::eResult::UnexpectedPID;
    } else if ((m_LastContinuityCounter==15 && PacketHeader->getContinuityCounter()!=0) || (m_LastContinuityCounter!=15 && (PacketHeader->getContinuityCounter() != (m_LastContinuityCounter+1))) && PacketHeader->getPayloadUntilStartIndicator() != 1){
        m_LastContinuityCounter=PacketHeader->getContinuityCounter();
        return xPES_Assembler::eResult::StreamPacketLost;
    } else{

        if(m_Started && PacketHeader->getPayloadUntilStartIndicator()){    // Finish to Start - Ensures that if Packet Ends without AFL
            m_Started = false;                                             // There will be a swift transition from end to beggining of a new Packet
            fwrite(m_Buffer, sizeof(char), m_DataOffset-m_PESH.getHeaderLength(), m_streamDataFile);
            delete m_Buffer;
            return xPES_Assembler::eResult::AssemblingFinishedToStart;
        }

        if(PacketHeader->hasAdaptationField()){
            TransportStreamPacket+=AdaptationField->getAdaptationFieldLength()+5;
        } else {
            TransportStreamPacket+=4;
        }

        m_LastContinuityCounter=PacketHeader->getContinuityCounter();
        uint8_t jmp;

        if(!m_Started && PacketHeader->getPayloadUntilStartIndicator()){   // Start
            m_Started=true;
            m_PESH.Reset();
            m_PESH.Parse(TransportStreamPacket);
            TransportStreamPacket+=m_PESH.getHeaderLength();
            m_DataOffset=xTS::TS_PacketLength-4;
            if(PacketHeader->hasAdaptationField()){
                m_DataOffset-=(AdaptationField->getAdaptationFieldLength()+1);
            }
            m_BufferSize=m_DataOffset-m_PESH.getHeaderLength();
            m_BufferPos=0;
            if(m_PESH.getPacketLength()){
                m_Buffer=new uint8_t[m_PESH.getPacketLength()-3-m_PESH.getExtendedHeaderDataLength()];
            } else{
                m_Buffer=new uint8_t[m_BufferSize];
            }
            while(m_BufferPos < m_BufferSize){
                m_Buffer[m_BufferPos]=TransportStreamPacket[m_BufferPos];
                m_BufferPos++;
            }
            return xPES_Assembler::eResult::AssemblingStarted;
        } else if(!PacketHeader->getPayloadUntilStartIndicator() && PacketHeader->getAdaptationFieldControl() == 3){            // Finish
            m_Started = false;
            m_BufferSize=183-AdaptationField->getAdaptationFieldLength();
            m_DataOffset+=m_BufferSize;
            if(!m_PESH.getPacketLength()){
                uint8_t* m_SwapBuffer = new uint8_t[m_BufferPos];
                
                for(uint32_t i=0; i<m_BufferPos; i++){
                    m_SwapBuffer[i]=m_Buffer[i];
                }
                delete m_Buffer; m_Buffer = new uint8_t[m_BufferPos+m_BufferSize];
                for(uint32_t i=0; i<m_BufferPos; i++){
                    m_Buffer[i]=m_SwapBuffer[i];
                }
                
                delete m_SwapBuffer;
            }
            uint32_t j=0;
            while(m_BufferPos<m_DataOffset-m_PESH.getHeaderLength()){
                m_Buffer[m_BufferPos]=TransportStreamPacket[j];
                j++; m_BufferPos++;
            }
            fwrite(m_Buffer, sizeof(char), m_DataOffset-m_PESH.getHeaderLength(), m_streamDataFile);
            delete m_Buffer;
            return xPES_Assembler::eResult::AssemblingFinished;
        } else{                                                                                                                 // Continue
            if(!PacketHeader->hasAdaptationField()){
                m_BufferSize=184;
            } else{
                m_BufferSize=183-AdaptationField->getAdaptationFieldLength();
            }
            m_DataOffset+=m_BufferSize;
            if(!m_PESH.getPacketLength()){
                uint8_t* m_SwapBuffer = new uint8_t[m_BufferPos];
                
                for(uint32_t i=0; i<m_BufferPos; i++){
                    m_SwapBuffer[i]=m_Buffer[i];
                }
                delete m_Buffer; m_Buffer = new uint8_t[m_BufferPos+m_BufferSize];
                for(uint32_t i=0; i<m_BufferPos; i++){
                    m_Buffer[i]=m_SwapBuffer[i];
                }
                
                delete m_SwapBuffer;
            }
            uint32_t j=0;
            while(m_BufferPos<m_DataOffset-m_PESH.getHeaderLength()){
                m_Buffer[m_BufferPos]=TransportStreamPacket[j];
                j++; m_BufferPos++;
            }
            return xPES_Assembler::eResult::AssemblingContinue;
        }

    }


}

void xPES_Assembler::ReadInputBinary8(uint8_t byte){

    printf("\t");

      for (uint8_t mask=128;mask;){
        if(mask&byte){
            printf("1");
        } else{
            printf("0");
        }

        mask=mask>>1;
    }


}


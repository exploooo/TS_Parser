#pragma once
#include "tsCommon.h"
#include <string>

/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================

class xTS
{
public:
  static constexpr uint32_t TS_PacketLength  = 188;
  static constexpr uint32_t TS_HeaderLength  = 4;

  static constexpr uint32_t PES_HeaderLength = 6;

  static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
  enum class ePID : uint16_t
  {
    PAT  = 0x0000,
    CAT  = 0x0001,
    TSDT = 0x0002,
    IPMT = 0x0003,
    NIT  = 0x0010, //DVB specific PID
    SDT  = 0x0011, //DVB specific PID
    NuLL = 0x1FFF,
  };

protected:
  uint8_t m_SB;
  bool m_E;
  bool m_S;
  bool m_T;
  uint16_t m_PID;
  uint8_t m_TSC;
  uint8_t m_AFC;
  uint8_t m_CC;

public:
  void     Reset();
  int32_t  Parse(const uint8_t* Input);
  void     Print() const;
  void     ReadInputBinary8(uint8_t byte);
  void     ReadInputBinary32(uint32_t byte);
  void     readPacketType() const;


public:
  uint8_t  getSyncByte() const { return m_SB; }
  uint8_t  getTransportErrorIndicator() const { return m_E; }
  uint8_t  getPayloadUntilStartIndicator() const { return m_S; }
  uint8_t  getTransportPriority() const { return m_T; }
  uint16_t  getPacketIdentifier() const { return m_PID; }
  uint8_t  getTransportScramblingControl() const { return m_TSC; }
  uint8_t  getAdaptationFieldControl() const { return m_AFC; }
  uint8_t  getContinuityCounter() const { return m_CC; }


public:
  bool     hasAdaptationField() const { if((m_AFC&0b11)==0b10 || (m_AFC&0b11)==0b11)return 1; else return 0; }
  bool     hasPayload        () const { if((m_AFC&0b11)==0b01 || (m_AFC&0b11)==0b11)return 1; else return 0; }

};

//=============================================================================================================================================================================

class xTS_AdaptationField
{
protected:
//setup
uint8_t m_AFC;
//mandatory fields
uint8_t m_AFL;
bool m_DC;
bool m_RA;
bool m_SP;
bool m_PR;
bool m_OR;
bool m_SF;
bool m_TP;
bool m_EX;
//optional fields - PCR
uint64_t m_PCRB;
uint16_t m_PCRE;
uint64_t m_PCR;
uint64_t m_lastPCR = 0;
//optional fields - OPCR
uint64_t m_OPCRB;
uint16_t m_OPCRE;
uint64_t m_OPCR;
//optional field - TP LENGTH
uint8_t m_TPLen;
//optional field - AFE LENGTH
uint8_t m_AFELen;

//Stuffing Bytes
uint32_t m_StuffingBytesCount;

public:
void Reset();
int32_t Parse(const uint8_t* PacketBuffer, uint8_t AFC);
void Print() const;
void ReadInputBinary8(uint8_t byte);

public:
//mandatory fields

uint8_t getAdaptationFieldLength () const { return m_AFL ; }
bool getDiscontinuityIndicator () const { return m_DC ; }
bool getRandomAccessIndicator () const { return m_RA ; }
bool getElementaryStreamPriorityIndicator () const { return m_SP ; }
bool getPCRFlag () const { return m_PR ; }
bool getOPCRFlag () const { return m_OR ; }
bool getSplicingPointFlag () const { return m_SF ; }
bool getTransportPrivateDataFlag () const { return m_TP ; }
bool getadAptationFieldExtensionFlag () const { return m_EX ; }

//derived
uint32_t getNumStuffingBytes () const { return m_StuffingBytesCount ; }
};

//=============================================================================================================================================================================

class xPES_PacketHeader
{
public:
  enum eStreamId : uint8_t
  {
    eStreamId_program_stream_map = 0xBC,
    eStreamId_padding_stream = 0xBE,
    eStreamId_private_stream_2 = 0xBF,
    eStreamId_ECM = 0xF0,
    eStreamId_EMM = 0xF1,
    eStreamId_program_stream_directory = 0xFF,
    eStreamId_DSMCC_stream = 0xF2,
    eStreamId_ITUT_H222_1_type_E = 0xF8,
  };

protected:
  //PES packet header
  uint32_t m_PacketStartCodePrefix;
  uint8_t m_StreamId;
  uint16_t m_PacketLength;
  int32_t m_HeaderLength;
  //PES extended header
  uint8_t m_PTS_DTS_Flags;
  uint32_t m_PTS;
  uint32_t m_DTS;
  uint8_t m_ExtendedHeaderDataLength;

public:
  void Reset();
  int32_t Parse(const uint8_t* Input);
  void Print() const;

public:
  //PES packet header
  uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
  int32_t getHeaderLength() const { return m_HeaderLength; }
  uint8_t getExtendedHeaderDataLength() const { return m_ExtendedHeaderDataLength; }
  uint8_t getStreamId () const { return m_StreamId; }
  uint16_t getPacketLength () const { return m_PacketLength; }
  void ReadInputBinary8(uint8_t byte);

};

//=============================================================================================================================================================================

class xPES_Assembler
{
public:
  enum class eResult : int32_t
  {
    UnexpectedPID = 1,
    StreamPacketLost ,
    AssemblingStarted ,
    AssemblingContinue,
    AssemblingFinished,
    AssemblingFinishedToStart
  };
protected:
  //setup
  int32_t m_PID;
  //buffer
  uint8_t* m_Buffer;
  uint32_t m_BufferSize;
  uint32_t m_BufferPos;
  uint32_t m_DataOffset;
  //operation
  int8_t m_LastContinuityCounter;
  bool m_Started;
  xPES_PacketHeader m_PESH;
  FILE *m_streamDataFile;

public:
  xPES_Assembler(){}
  ~xPES_Assembler(){ fclose(m_streamDataFile); }
  void Init (int32_t PID);
  eResult AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField);
  void PrintPESH () const { m_PESH.Print(); }
  uint8_t* getPacket () { return m_Buffer; }
  int32_t getNumPacketBytes() const { return m_DataOffset; }
  int32_t getHeaderLength() const { return m_PESH.getHeaderLength(); }
  void ReadInputBinary8(uint8_t byte);
  void switchStarted(){ m_Started = !m_Started; }

protected:
  void xBufferReset ();
  void xBufferAppend(const uint8_t* Data, int32_t Size);
};

//=============================================================================================================================================================================

/* Static utility class for reading and writing stream binary */
#include <string>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define _U32  ReadUInt32(*m_pDataStream)
#define _S32  ReadSInt32(*m_pDataStream)
#define _U16  ReadUShort(*m_pDataStream)
#define _BOOL ReadBool(*m_pDataStream)
#define _FLOAT ReadFloat(*m_pDataStream)

namespace BinaryIO {

	std::string ReadString(istream& fs, int size);
	int ReadInt(istream& fs);
	unsigned int ReadUInt(istream& in);
	uint32_t ReadUInt32(istream& in);
	int8_t ReadSInt8(istream& in);
	int16_t ReadSInt16(istream& in);
	int32_t ReadSInt32(istream& in);
	uint64_t ReadUInt64(istream& in);
	unsigned short ReadUShort(istream& in);
	short ReadShort(istream& in);
	uint8_t ReadByte(istream& in);
	float ReadFloat(istream& in);
	bool ReadBool(istream& in);

    void WriteUInt64(stringstream& ss, uint64_t value);
    void WriteUInt32(std::stringstream& ss, uint32_t value);
    void WriteUInt16(stringstream& ss, uint16_t value);
    void WriteByte(stringstream& ss, uint8_t value);
    void WriteBool(stringstream& ss, bool flag);
    void WriteInt32(stringstream& ss, int32_t value);
    void WriteFloat(stringstream& ss, float value);
    void WriteString(stringstream& ss, const std::string& string);
    void WriteChars(stringstream& ss, const std::string& value);
    void WriteSignature(stringstream& ss, const std::string& value);

    void WriteUInt64(ofstream* fs, uint64_t value);
    void WriteUInt32(ofstream* fs, uint32_t value);
    void WriteUInt16(ofstream* fs, uint16_t value);
    void WriteByte(ofstream* fs, uint8_t value);
    void WriteBool(ofstream* fs, bool flag);
    void WriteInt32(ofstream* fs, int32_t value);
    void WriteFloat(ofstream* fs, float value);
    void WriteString(ofstream* fs, std::string string);
    void WriteChars(ofstream* fs, std::string value);
	void WriteSignature(ofstream* fs, std::string value);

}

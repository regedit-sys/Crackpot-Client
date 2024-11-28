#pragma once
class MemoryBuffer
{
public:

	MemoryBuffer( DWORD dwSize = 512 )
	{
		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;

		if( ( dwSize < UINT_MAX ) && ( dwSize != 0 ) )
		{
			m_pBuffer = ( BYTE* )Native::Malloc( dwSize + 1 );    // one more char, in case when using string funcions
			if( m_pBuffer )
			{
				m_dwBufferSize = dwSize;
				m_pBuffer[0] = 0;
			}
		}
	};

	~MemoryBuffer()
	{
		if( m_pBuffer )
			Native::Free( m_pBuffer );

		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;
	};

    // Add chunk of memory to buffer
    BOOL    Add( const void* p, DWORD dwSize )
    {
        if( CheckSize( dwSize ) )
        {
            memcpy( m_pBuffer + m_dwDataLength, p, dwSize );
            m_dwDataLength += dwSize;
            *( m_pBuffer + m_dwDataLength ) = 0;    // fill end zero
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    };

    // Get the data in buffer
    BYTE* GetData() const
    {
        return m_pBuffer;
    };

    // Get the length of data in buffer
    DWORD   GetDataLength() const
    {
        return m_dwDataLength;
    };

    // Rewind the data pointer to the begining
    void    Rewind()
    {
        m_dwDataLength = 0; m_pBuffer[ 0 ] = 0;
    };

    // Automatically adjust increase buffer size if necessary
    BOOL    CheckSize( DWORD dwSize )
    {
        if( m_dwBufferSize >= ( m_dwDataLength + dwSize ) )
        {
            return TRUE;    // Enough space
        }
        else
        {
            // Try to double it
            DWORD dwNewSize = max( m_dwDataLength + dwSize, m_dwBufferSize * 2 );
            BYTE* pNewBuffer = ( UCHAR* )realloc( m_pBuffer, dwNewSize + 1 );        // one more char
            if( pNewBuffer )
            {
                m_pBuffer = pNewBuffer;
                m_dwBufferSize = dwNewSize;
                return TRUE;
            }
            else
            {
                // Failed
                return FALSE;
            }
        }
    }

	private:

	BYTE* m_pBuffer;

    DWORD m_dwDataLength;

    DWORD m_dwBufferSize;
};

HRESULT CreateSymbolicLink(const char *szDrive, const std::string szDeviceName, BOOL System);
HRESULT deleteSymbolicLink(const char* szDrive, BOOL System);
BOOL cReadFile(const char * FileName, MemoryBuffer &pBuffer);
BOOL cWriteFile(const char* FilePath, const VOID* Data, DWORD Size);
BOOL DirectoryExists(LPCSTR lpDirectoryName);
BOOL fileExists(LPCSTR lpFileName);
DWORD getDeviceSize(const std::string &device);
NTSTATUS getLowPartAllocationUnits(const char* device, PDWORD dest);
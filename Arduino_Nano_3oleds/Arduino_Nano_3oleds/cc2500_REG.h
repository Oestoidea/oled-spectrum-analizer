/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with C-code
 *  compatible #define statements.
 *
 *  RF device: CC2500
 *
 ***************************************************************/
// configuration registers
#define IOCFG2           0x0000  // GDO2 output pin configuration
#define IOCFG1           0x0001  // GDO1 output pin configuration
#define IOCFG0           0x0002  // GDO0 output pin configuration
#define FIFOTHR          0x0003  // RX FIFO and TX FIFO thresholds
#define SYNC1            0x0004  // Sync word, high byte
#define SYNC0            0x0005  // Sync word, low byte
#define PKTLEN           0x0006  // Packet length
#define PKTCTRL1         0x0007  // Packet automation control
#define PKTCTRL0         0x0008  // Packet automation control
#define ADDR             0x0009  // Device address
#define CHANNR           0x000A  // Channel number
#define FSCTRL1          0x000B  // Frequency synthesizer control
#define FSCTRL0          0x000C  // Frequency synthesizer control
#define FREQ2            0x000D  // Frequency control word, high byte
#define FREQ1            0x000E  // Frequency control word, middle byte
#define FREQ0            0x000F  // Frequency control word, low byte
#define MDMCFG4          0x0010  // Modem configuration
#define MDMCFG3          0x0011  // Modem configuration
#define MDMCFG2          0x0012  // Modem configuration
#define MDMCFG1          0x0013  // Modem configuration
#define MDMCFG0          0x0014  // Modem configuration
#define DEVIATN          0x0015  // Modem deviation setting
#define MCSM2            0x0016  // Main Radio Control State Machine configuration
#define MCSM1            0x0017  // Main Radio Control State Machine configuration
#define MCSM0            0x0018  // Main Radio Control State Machine configuration
#define FOCCFG           0x0019  // Frequency Offset Compensation configuration
#define BSCFG            0x001A  // Bit Synchronization configuration
#define AGCCTRL2         0x001B  // AGC control
#define AGCCTRL1         0x001C  // AGC control
#define AGCCTRL0         0x001D  // AGC control
#define WOREVT1          0x001E  // High byte Event 0 timeout
#define WOREVT0          0x001F  // Low byte Event 0 timeout
#define WORCTRL          0x0020  // Wake On Radio control
#define FREND1           0x0021  // Front end RX configuration
#define FREND0           0x0022  // Front end TX configuration
#define FSCAL3           0x0023  // Frequency synthesizer calibration
#define FSCAL2           0x0024  // Frequency synthesizer calibration
#define FSCAL1           0x0025  // Frequency synthesizer calibration
#define FSCAL0           0x0026  // Frequency synthesizer calibration
#define RCCTRL1          0x0027  // RC oscillator configuration
#define RCCTRL0          0x0028  // RC oscillator configuration
#define FSTEST           0x0029  // Frequency synthesizer calibration control
#define PTEST            0x002A  // Production test
#define AGCTEST          0x002B  // AGC test
#define TEST2            0x002C  // Various test settings
#define TEST1            0x002D  // Various test settings
#define TEST0            0x002E  // Various test settings

// command strobes
#define SRES             0x0030  // Reset chip
#define SFSTXON          0x0031  // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1)
#define SX0FF            0x0032  // Turn off crystal oscillator
#define SCAL             0x0033  // Calibrate frequency synthesizer and turn it off
#define SRX              0x0034  // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
#define STX              0x0035  // see datasheet p.57
#define SIDLE            0x0036  // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define SWOR             0x0038  // Start automatic RX polling sequence (Wake-on-Radio)
#define SPWD             0x0039  // Enter power down mode when CSn goes high
#define SFRX             0x003A  // Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states.
#define SFTX             0x003B  // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
#define SWORRST          0x003C  // Reset real time clock to Event1 value
#define SNOP             0x003F  // No operation. May be used to get access to the chip status byte

// status registers
#define PARTNUM          0x00F0  // CC2500 part number (80 after reset)
#define VERSION          0x00F1  // Current version number (03 after reset)
#define FREQEST          0x00F2  // Frequency offset estimate
#define LQI              0x00F3  // Demodulator estimate for Link Quality
#define REG_RSSI         0x00F4  // received signal strength indication
#define MARCSTATE        0x00F5  // Control state machine state
#define WORTIME1         0x00F6  // High byte of WOR timer
#define WORTIME0         0x00F7  // Low byte of WOR timer
#define PKTSTATUS        0x00F8  // Current GDOx status and packet status
#define VCO_VC_DAC       0x00F9  // Current setting from PLL calibration module
#define TXBYTES          0x00FA  // Underflow and number of bytes in the TX FIFO
#define RXBYTES          0x00FB  // Overflow and number of bytes in the RX FIFO
#define RCCTRL1_STATUS   0x00FC  // Last RC oscillator calibration result
#define RCCTRL0_STATUS   0x00FD  // Last RC oscillator calibration result

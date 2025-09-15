#include "DeviceInfo.hpp"

const DeviceInfo::IrqTable irq_table = {
    {-14, "NonMaskableInt"},
    {-12, "MemoryManagement"},
    {-11, "BusFault"},
    {-10, "UsageFault"},
    {-5, "SVCall"},
    {-4, "DebugMonitor"},
    {-2, "PendSV"},
    {-1, "SysTick"},
    {0, "WWDGT_IRQ"},
    {1, "LVD_IRQ"},
    {2, "TAMPER_STAMP_IRQ"},
    {3, "RTC_WKUP_IRQ"},
    {4, "FMC_IRQ"},
    {5, "RCU_CTC_IRQ"},
    {6, "EXTI0_IRQ"},
    {7, "EXTI1_IRQ"},
    {8, "EXTI2_IRQ"},
    {9, "EXTI3_IRQ"},
    {10, "EXTI4_IRQ"},
    {11, "DMA0_Channel0_IRQ"},
    {12, "DMA0_Channel1_IRQ"},
    {13, "DMA0_Channel2_IRQ"},
    {14, "DMA0_Channel3_IRQ"},
    {15, "DMA0_Channel4_IRQ"},
    {16, "DMA0_Channel5_IRQ"},
    {17, "DMA0_Channel6_IRQ"},
    {18, "ADC_IRQ"},
    {19, "CAN0_TX_IRQ"},
    {20, "CAN0_RX0_IRQ"},
    {21, "CAN0_RX1_IRQ"},
    {22, "CAN0_EWMC_IRQ"},
    {23, "EXTI5_9_IRQ"},
    {24, "TIMER0_BRK_TIMER8_IRQ"},
    {25, "TIMER0_UP_TIMER9_IRQ"},
    {26, "TIMER0_TRG_CMT_TIMER10_IRQ"},
    {27, "TIMER0_CC_IRQ"},
    {28, "TIMER1_IRQ"},
    {29, "TIMER2_IRQ"},
    {30, "TIMER3_IRQ"},
    {31, "I2C0_EV_IRQ"},
    {32, "I2C0_ER_IRQ"},
    {33, "I2C1_EV_IRQ"},
    {34, "I2C1_ER_IRQ"},
    {35, "SPI0_IRQ"},
    {36, "SPI1_IRQ"},
    {37, "USART0_IRQ"},
    {38, "USART1_IRQ"},
    {39, "USART2_IRQ"},
    {40, "EXTI10_15_IRQ"},
    {41, "RTC_Alarm_IRQ"},
    {42, "USBFS_WKUP_IRQ"},
    {43, "TIMER7_BRK_TIMER11_IRQ"},
    {44, "TIMER7_UP_TIMER12_IRQ"},
    {45, "TIMER7_TRG_CMT_TIMER13_IRQ"},
    {46, "TIMER7_CC_IRQ"},
    {47, "DMA0_Channel7_IRQ"},
    {48, "EXMC_IRQ"},
    {49, "SDIO_IRQ"},
    {50, "TIMER4_IRQ"},
    {51, "SPI2_IRQ"},
    {52, "UART3_IRQ"},
    {53, "UART4_IRQ"},
    {54, "TIMER5_DAC_IRQ"},
    {55, "TIMER6_IRQ"},
    {56, "DMA1_Channel0_IRQ"},
    {57, "DMA1_Channel1_IRQ"},
    {58, "DMA1_Channel2_IRQ"},
    {59, "DMA1_Channel3_IRQ"},
    {60, "DMA1_Channel4_IRQ"},
    {61, "ENET_IRQ"},
    {62, "ENET_WKUP_IRQ"},
    {63, "CAN1_TX_IRQ"},
    {64, "CAN1_RX0_IRQ"},
    {65, "CAN1_RX1_IRQ"},
    {66, "CAN1_EWMC_IRQ"},
    {67, "USBFS_IRQ"},
    {68, "DMA1_Channel5_IRQ"},
    {69, "DMA1_Channel6_IRQ"},
    {70, "DMA1_Channel7_IRQ"},
    {71, "USART5_IRQ"},
    {72, "I2C2_EV_IRQ"},
    {73, "I2C2_ER_IRQ"},
    {74, "USBHS_EP1_Out_IRQ"},
    {75, "USBHS_EP1_In_IRQ"},
    {76, "USBHS_WKUP_IRQ"},
    {77, "USBHS_IRQ"},
    {78, "DCI_IRQ"},
    {80, "TRNG_IRQ"},
    {81, "FPU_IRQ"},
    {82, "UART6_IRQ"},
    {83, "UART7_IRQ"},
    {84, "SPI3_IRQ"},
    {85, "SPI4_IRQ"},
    {86, "SPI5_IRQ"},
    {88, "TLI_IRQ"},
    {89, "TLI_ER_IRQ"},
    {90, "IPA_IRQ"},
};

// SDIO_INT_FLAG_CCRCERR           BIT(0)
// SDIO_INT_FLAG_DTCRCERR          BIT(1)
// SDIO_INT_FLAG_CMDTMOUT          BIT(2)
// SDIO_INT_FLAG_DTTMOUT           BIT(3)
// SDIO_INT_FLAG_TXURE             BIT(4)
// SDIO_INT_FLAG_RXORE             BIT(5)
// SDIO_INT_FLAG_CMDRECV           BIT(6)
// SDIO_INT_FLAG_CMDSEND           BIT(7)
// SDIO_INT_FLAG_DTEND             BIT(8)
// SDIO_INT_FLAG_STBITE            BIT(9)
// SDIO_INT_FLAG_DTBLKEND          BIT(10)
// SDIO_INT_FLAG_CMDRUN            BIT(11)
// SDIO_INT_FLAG_TXRUN             BIT(12)
// SDIO_INT_FLAG_RXRUN             BIT(13)
// SDIO_INT_FLAG_TFH               BIT(14)
// SDIO_INT_FLAG_RFH               BIT(15)
// SDIO_INT_FLAG_TFF               BIT(16)
// SDIO_INT_FLAG_RFF               BIT(17)
// SDIO_INT_FLAG_TFE               BIT(18)
// SDIO_INT_FLAG_RFE               BIT(19)
// SDIO_INT_FLAG_TXDTVAL           BIT(20)
// SDIO_INT_FLAG_RXDTVAL           BIT(21)
// SDIO_INT_FLAG_SDIOINT           BIT(22)
// SDIO_INT_FLAG_ATAEND            BIT(23)

// {BIT(0), DMA_INTF_FEEIF}                                            /*!< FIFO error and exception flag */
// {BIT(2), DMA_INTF_SDEIF}                                            /*!< single data mode exception flag */
// {BIT(3), DMA_INTF_TAEIF}                                            /*!< transfer access error flag */
// {BIT(4), DMA_INTF_HTFIF}                                            /*!< half transfer finish flag */
// {BIT(5), DMA_INTF_FTFIF}                                            /*!< full transger finish flag */

const DeviceInfo device_info_gd32f4xx = DeviceInfo{irq_table};
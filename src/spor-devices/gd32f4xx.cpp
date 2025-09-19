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
    {0, "WWDGT_IRQHandler"},
    {1, "LVD_IRQHandler"},
    {2, "TAMPER_STAMP_IRQHandler"},
    {3, "RTC_WKUP_IRQHandler"},
    {4, "FMC_IRQHandler"},
    {5, "RCU_CTC_IRQHandler"},
    {6, "EXTI0_IRQHandler"},
    {7, "EXTI1_IRQHandler"},
    {8, "EXTI2_IRQHandler"},
    {9, "EXTI3_IRQHandler"},
    {10, "EXTI4_IRQHandler"},
    {11, "DMA0_Channel0_IRQHandler"},
    {12, "DMA0_Channel1_IRQHandler"},
    {13, "DMA0_Channel2_IRQHandler"},
    {14, "DMA0_Channel3_IRQHandler"},
    {15, "DMA0_Channel4_IRQHandler"},
    {16, "DMA0_Channel5_IRQHandler"},
    {17, "DMA0_Channel6_IRQHandler"},
    {18, "ADC_IRQHandler"},
    {19, "CAN0_TX_IRQHandler"},
    {20, "CAN0_RX0_IRQHandler"},
    {21, "CAN0_RX1_IRQHandler"},
    {22, "CAN0_EWMC_IRQHandler"},
    {23, "EXTI5_9_IRQHandler"},
    {24, "TIMER0_BRK_TIMER8_IRQHandler"},
    {25, "TIMER0_UP_TIMER9_IRQHandler"},
    {26, "TIMER0_TRG_CMT_TIMER10_IRQHandler"},
    {27, "TIMER0_CC_IRQHandler"},
    {28, "TIMER1_IRQHandler"},
    {29, "TIMER2_IRQHandler"},
    {30, "TIMER3_IRQHandler"},
    {31, "I2C0_EV_IRQHandler"},
    {32, "I2C0_ER_IRQHandler"},
    {33, "I2C1_EV_IRQHandler"},
    {34, "I2C1_ER_IRQHandler"},
    {35, "SPI0_IRQHandler"},
    {36, "SPI1_IRQHandler"},
    {37, "USART0_IRQHandler"},
    {38, "USART1_IRQHandler"},
    {39, "USART2_IRQHandler"},
    {40, "EXTI10_15_IRQHandler"},
    {41, "RTC_Alarm_IRQHandler"},
    {42, "USBFS_WKUP_IRQHandler"},
    {43, "TIMER7_BRK_TIMER11_IRQHandler"},
    {44, "TIMER7_UP_TIMER12_IRQHandler"},
    {45, "TIMER7_TRG_CMT_TIMER13_IRQHandler"},
    {46, "TIMER7_CC_IRQHandler"},
    {47, "DMA0_Channel7_IRQHandler"},
    {48, "EXMC_IRQHandler"},
    {49, "SDIO_IRQHandler"},
    {50, "TIMER4_IRQHandler"},
    {51, "SPI2_IRQHandler"},
    {52, "UART3_IRQHandler"},
    {53, "UART4_IRQHandler"},
    {54, "TIMER5_DAC_IRQHandler"},
    {55, "TIMER6_IRQHandler"},
    {56, "DMA1_Channel0_IRQHandler"},
    {57, "DMA1_Channel1_IRQHandler"},
    {58, "DMA1_Channel2_IRQHandler"},
    {59, "DMA1_Channel3_IRQHandler"},
    {60, "DMA1_Channel4_IRQHandler"},
    {61, "ENET_IRQHandler"},
    {62, "ENET_WKUP_IRQHandler"},
    {63, "CAN1_TX_IRQHandler"},
    {64, "CAN1_RX0_IRQHandler"},
    {65, "CAN1_RX1_IRQHandler"},
    {66, "CAN1_EWMC_IRQHandler"},
    {67, "USBFS_IRQHandler"},
    {68, "DMA1_Channel5_IRQHandler"},
    {69, "DMA1_Channel6_IRQHandler"},
    {70, "DMA1_Channel7_IRQHandler"},
    {71, "USART5_IRQHandler"},
    {72, "I2C2_EV_IRQHandler"},
    {73, "I2C2_ER_IRQHandler"},
    {74, "USBHS_EP1_Out_IRQHandler"},
    {75, "USBHS_EP1_In_IRQHandler"},
    {76, "USBHS_WKUP_IRQHandler"},
    {77, "USBHS_IRQHandler"},
    {78, "DCI_IRQHandler"},
    {80, "TRNG_IRQHandler"},
    {81, "FPU_IRQHandler"},
    {82, "UART6_IRQHandler"},
    {83, "UART7_IRQHandler"},
    {84, "SPI3_IRQHandler"},
    {85, "SPI4_IRQHandler"},
    {86, "SPI5_IRQHandler"},
    {88, "TLI_IRQHandler"},
    {89, "TLI_ER_IRQHandler"},
    {90, "IPA_IRQHandler"},
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
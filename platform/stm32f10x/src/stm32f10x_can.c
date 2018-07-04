/**
* This file is part of the auto-elevator project.
*
* Copyright 2018, Huang Yang <elious.huang@gmail.com>. All rights reserved.
*
* See the COPYING file for the terms of usage and distribution.
*/
#include "stm32f10x_can.h"
#include "stm32f10x_map.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_cfg.h"


/* can tx mailbox register */
typedef struct
{
    volatile uint32_t TIR;
    volatile uint32_t TDTR;
    volatile uint32_t TDLR;
    volatile uint32_t TDHR;
}CAN_TxMailBox;

/* can fifo mailbox register */
typedef struct
{
    volatile uint32_t RIR;
    volatile uint32_t RDTR;
    volatile uint32_t RDLR;
    volatile uint32_t RDHR;
}CAN_FIFOMailBox;

/* can filter register */ 
typedef struct
{
    volatile uint32_t FR1;
    volatile uint32_t FR2;
}CAN_FilterRegister;

/* can register structure */
typedef struct
{
    volatile uint32_t MCR;
    volatile uint32_t MSR;
    volatile uint32_t TSR;
    volatile uint32_t RF0R;
    volatile uint32_t RF1R;
    volatile uint32_t IER;
    volatile uint32_t ESR;
    volatile uint32_t BTR;
    uint32_t  RESERVED0[88];
    CAN_TxMailBox TxMailBox[3];
    CAN_FIFOMailBox FIFOMailBox[2];
    uint32_t  RESERVED1[12];
    volatile uint32_t FMR;
    volatile uint32_t FM1R;
    uint32_t  RESERVED2;
    volatile uint32_t FS1R;
    uint32_t  RESERVED3;
    volatile uint32_t FFA1R;
    uint32_t  RESERVED4;
    volatile uint32_t FA1R;
    uint32_t  RESERVED5[8];
    CAN_FilterRegister FilterRegister[28]; 
} CAN_T;

/* CAN group array */
static CAN_T * const CANX[] = {(CAN_T *)BxCAN1_BASE, (CAN_T *)BxCAN2_BASE};


/* MCR register */
#define  CAN_MCR_INRQ      ((uint16_t)0x0001)
#define  CAN_MCR_SLEEP     ((uint16_t)0x0002)
#define  CAN_MCR_TXFP      ((uint16_t)0x0004)
#define  CAN_MCR_RFLM      ((uint16_t)0x0008)
#define  CAN_MCR_NART      ((uint16_t)0x0010)
#define  CAN_MCR_AWUM      ((uint16_t)0x0020)
#define  CAN_MCR_ABOM      ((uint16_t)0x0040)
#define  CAN_MCR_TTCM      ((uint16_t)0x0080)
#define  CAN_MCR_RESET     ((uint16_t)0x8000)

/* MSR register */
#define  CAN_MSR_INAK      ((uint16_t)0x0001)
#define  CAN_MSR_SLAK      ((uint16_t)0x0002)
#define  CAN_MSR_ERRI      ((uint16_t)0x0004)
#define  CAN_MSR_WKUI      ((uint16_t)0x0008)
#define  CAN_MSR_SLAKI     ((uint16_t)0x0010)
#define  CAN_MSR_TXM       ((uint16_t)0x0100)
#define  CAN_MSR_RXM       ((uint16_t)0x0200)
#define  CAN_MSR_SAMP      ((uint16_t)0x0400)
#define  CAN_MSR_RX        ((uint16_t)0x0800)

/* TSR register */
#define  CAN_TSR_RQCP0     ((uint32_t)0x00000001)
#define  CAN_TSR_TXOK0     ((uint32_t)0x00000002)
#define  CAN_TSR_ALST0     ((uint32_t)0x00000004)
#define  CAN_TSR_TERR0     ((uint32_t)0x00000008)
#define  CAN_TSR_ABRQ0     ((uint32_t)0x00000080)
#define  CAN_TSR_RQCP1     ((uint32_t)0x00000100)
#define  CAN_TSR_TXOK1     ((uint32_t)0x00000200)
#define  CAN_TSR_ALST1     ((uint32_t)0x00000400)
#define  CAN_TSR_TERR1     ((uint32_t)0x00000800)
#define  CAN_TSR_ABRQ1     ((uint32_t)0x00008000)
#define  CAN_TSR_RQCP2     ((uint32_t)0x00010000)
#define  CAN_TSR_TXOK2     ((uint32_t)0x00020000)
#define  CAN_TSR_ALST2     ((uint32_t)0x00040000)
#define  CAN_TSR_TERR2     ((uint32_t)0x00080000)
#define  CAN_TSR_ABRQ2     ((uint32_t)0x00800000)
#define  CAN_TSR_CODE      ((uint32_t)0x03000000)

#define  CAN_TSR_TME       ((uint32_t)0x1C000000)
#define  CAN_TSR_TME0      ((uint32_t)0x04000000)
#define  CAN_TSR_TME1      ((uint32_t)0x08000000)
#define  CAN_TSR_TME2      ((uint32_t)0x10000000)

#define  CAN_TSR_LOW       ((uint32_t)0xE0000000)
#define  CAN_TSR_LOW0      ((uint32_t)0x20000000)
#define  CAN_TSR_LOW1      ((uint32_t)0x40000000)
#define  CAN_TSR_LOW2      ((uint32_t)0x80000000)

/* RF0R register */
#define  CAN_RF0R_FMP0     ((uint8_t)0x03)
#define  CAN_RF0R_FULL0    ((uint8_t)0x08)
#define  CAN_RF0R_FOVR0    ((uint8_t)0x10)
#define  CAN_RF0R_RFOM0    ((uint8_t)0x20)

/* RF1R register */
#define  CAN_RF1R_FMP1     ((uint8_t)0x03)
#define  CAN_RF1R_FULL1    ((uint8_t)0x08)
#define  CAN_RF1R_FOVR1    ((uint8_t)0x10)
#define  CAN_RF1R_RFOM1    ((uint8_t)0x20)

/* IER register */
#define  CAN_IER_TMEIE     ((uint32_t)0x00000001)
#define  CAN_IER_FMPIE0    ((uint32_t)0x00000002)
#define  CAN_IER_FFIE0     ((uint32_t)0x00000004)
#define  CAN_IER_FOVIE0    ((uint32_t)0x00000008)
#define  CAN_IER_FMPIE1    ((uint32_t)0x00000010)
#define  CAN_IER_FFIE1     ((uint32_t)0x00000020)
#define  CAN_IER_FOVIE1    ((uint32_t)0x00000040)
#define  CAN_IER_EWGIE     ((uint32_t)0x00000100)
#define  CAN_IER_EPVIE     ((uint32_t)0x00000200)
#define  CAN_IER_BOFIE     ((uint32_t)0x00000400)
#define  CAN_IER_LECIE     ((uint32_t)0x00000800)
#define  CAN_IER_ERRIE     ((uint32_t)0x00008000)
#define  CAN_IER_WKUIE     ((uint32_t)0x00010000)
#define  CAN_IER_SLKIE     ((uint32_t)0x00020000)

/* ESR register */
#define  CAN_ESR_EWGF      ((uint32_t)0x00000001)
#define  CAN_ESR_EPVF      ((uint32_t)0x00000002)
#define  CAN_ESR_BOFF      ((uint32_t)0x00000004)

#define  CAN_ESR_LEC       ((uint32_t)0x00000070)
#define  CAN_ESR_LEC_0     ((uint32_t)0x00000010)
#define  CAN_ESR_LEC_1     ((uint32_t)0x00000020)
#define  CAN_ESR_LEC_2     ((uint32_t)0x00000040)

#define  CAN_ESR_TEC       ((uint32_t)0x00FF0000)
#define  CAN_ESR_REC       ((uint32_t)0xFF000000)

/* BTR register */
#define  CAN_BTR_BRP       ((uint32_t)0x000003FF)
#define  CAN_BTR_TS1       ((uint32_t)0x000F0000)
#define  CAN_BTR_TS2       ((uint32_t)0x00700000)
#define  CAN_BTR_SJW       ((uint32_t)0x03000000)
#define  CAN_BTR_LBKM      ((uint32_t)0x40000000)
#define  CAN_BTR_SILM      ((uint32_t)0x80000000)

/* TI0R register */
#define  CAN_TI0R_TXRQ     ((uint32_t)0x00000001)
#define  CAN_TI0R_RTR      ((uint32_t)0x00000002)
#define  CAN_TI0R_IDE      ((uint32_t)0x00000004)
#define  CAN_TI0R_EXID     ((uint32_t)0x001FFFF8)
#define  CAN_TI0R_STID     ((uint32_t)0xFFE00000)

/* TDT0R register */
#define  CAN_TDT0R_DLC     ((uint32_t)0x0000000F)
#define  CAN_TDT0R_TGT     ((uint32_t)0x00000100)
#define  CAN_TDT0R_TIME    ((uint32_t)0xFFFF0000)

/* TDL0R register */
#define  CAN_TDL0R_DATA0   ((uint32_t)0x000000FF)
#define  CAN_TDL0R_DATA1   ((uint32_t)0x0000FF00)
#define  CAN_TDL0R_DATA2   ((uint32_t)0x00FF0000)
#define  CAN_TDL0R_DATA3   ((uint32_t)0xFF000000)

/* TDH0R register */
#define  CAN_TDH0R_DATA4   ((uint32_t)0x000000FF)
#define  CAN_TDH0R_DATA5   ((uint32_t)0x0000FF00)
#define  CAN_TDH0R_DATA6   ((uint32_t)0x00FF0000)
#define  CAN_TDH0R_DATA7   ((uint32_t)0xFF000000)

/* TI1R register */
#define  CAN_TI1R_TXRQ     ((uint32_t)0x00000001)
#define  CAN_TI1R_RTR      ((uint32_t)0x00000002)
#define  CAN_TI1R_IDE      ((uint32_t)0x00000004)
#define  CAN_TI1R_EXID     ((uint32_t)0x001FFFF8)
#define  CAN_TI1R_STID     ((uint32_t)0xFFE00000)

/* TDT1R register */ 
#define  CAN_TDT1R_DLC     ((uint32_t)0x0000000F)
#define  CAN_TDT1R_TGT     ((uint32_t)0x00000100)
#define  CAN_TDT1R_TIME    ((uint32_t)0xFFFF0000)

/* TDL1R register **/
#define  CAN_TDL1R_DATA0   ((uint32_t)0x000000FF)
#define  CAN_TDL1R_DATA1   ((uint32_t)0x0000FF00)
#define  CAN_TDL1R_DATA2   ((uint32_t)0x00FF0000)
#define  CAN_TDL1R_DATA3   ((uint32_t)0xFF000000)

/* TDH1R register */
#define  CAN_TDH1R_DATA4   ((uint32_t)0x000000FF)
#define  CAN_TDH1R_DATA5   ((uint32_t)0x0000FF00)
#define  CAN_TDH1R_DATA6   ((uint32_t)0x00FF0000)
#define  CAN_TDH1R_DATA7   ((uint32_t)0xFF000000)

/* TI2R register */
#define  CAN_TI2R_TXRQ     ((uint32_t)0x00000001)
#define  CAN_TI2R_RTR      ((uint32_t)0x00000002)
#define  CAN_TI2R_IDE      ((uint32_t)0x00000004)
#define  CAN_TI2R_EXID     ((uint32_t)0x001FFFF8)
#define  CAN_TI2R_STID     ((uint32_t)0xFFE00000)

/* TDT2R register */  
#define  CAN_TDT2R_DLC     ((uint32_t)0x0000000F)
#define  CAN_TDT2R_TGT     ((uint32_t)0x00000100)
#define  CAN_TDT2R_TIME    ((uint32_t)0xFFFF0000)

/* TDL2R register */
#define  CAN_TDL2R_DATA0   ((uint32_t)0x000000FF)
#define  CAN_TDL2R_DATA1   ((uint32_t)0x0000FF00)
#define  CAN_TDL2R_DATA2   ((uint32_t)0x00FF0000)
#define  CAN_TDL2R_DATA3   ((uint32_t)0xFF000000)

/* TDH2R register */
#define  CAN_TDH2R_DATA4   ((uint32_t)0x000000FF)
#define  CAN_TDH2R_DATA5   ((uint32_t)0x0000FF00)
#define  CAN_TDH2R_DATA6   ((uint32_t)0x00FF0000)
#define  CAN_TDH2R_DATA7   ((uint32_t)0xFF000000)

/* RI0R register */
#define  CAN_RI0R_RTR      ((uint32_t)0x00000002)
#define  CAN_RI0R_IDE      ((uint32_t)0x00000004)
#define  CAN_RI0R_EXID     ((uint32_t)0x001FFFF8)
#define  CAN_RI0R_STID     ((uint32_t)0xFFE00000)

/* RDT0R register */
#define  CAN_RDT0R_DLC     ((uint32_t)0x0000000F)
#define  CAN_RDT0R_FMI     ((uint32_t)0x0000FF00)
#define  CAN_RDT0R_TIME    ((uint32_t)0xFFFF0000)

/* RDL0R register */
#define  CAN_RDL0R_DATA0   ((uint32_t)0x000000FF)
#define  CAN_RDL0R_DATA1   ((uint32_t)0x0000FF00)
#define  CAN_RDL0R_DATA2   ((uint32_t)0x00FF0000)
#define  CAN_RDL0R_DATA3   ((uint32_t)0xFF000000)

/* RDH0R register */
#define  CAN_RDH0R_DATA4   ((uint32_t)0x000000FF)
#define  CAN_RDH0R_DATA5   ((uint32_t)0x0000FF00)
#define  CAN_RDH0R_DATA6   ((uint32_t)0x00FF0000)
#define  CAN_RDH0R_DATA7   ((uint32_t)0xFF000000)

/* RI1R register */
#define  CAN_RI1R_RTR      ((uint32_t)0x00000002)
#define  CAN_RI1R_IDE      ((uint32_t)0x00000004)
#define  CAN_RI1R_EXID     ((uint32_t)0x001FFFF8)
#define  CAN_RI1R_STID     ((uint32_t)0xFFE00000)

/* RDT1R register */
#define  CAN_RDT1R_DLC     ((uint32_t)0x0000000F)
#define  CAN_RDT1R_FMI     ((uint32_t)0x0000FF00)
#define  CAN_RDT1R_TIME    ((uint32_t)0xFFFF0000)

/* RDL1R register */
#define  CAN_RDL1R_DATA0   ((uint32_t)0x000000FF)
#define  CAN_RDL1R_DATA1   ((uint32_t)0x0000FF00)
#define  CAN_RDL1R_DATA2   ((uint32_t)0x00FF0000)
#define  CAN_RDL1R_DATA3   ((uint32_t)0xFF000000)

/* RDH1R register */
#define  CAN_RDH1R_DATA4   ((uint32_t)0x000000FF)
#define  CAN_RDH1R_DATA5   ((uint32_t)0x0000FF00)
#define  CAN_RDH1R_DATA6   ((uint32_t)0x00FF0000)
#define  CAN_RDH1R_DATA7   ((uint32_t)0xFF000000)



#define MCR_DBF      ((uint32_t)0x00010000) /* software master reset */

/* CAN Mailbox Transmit Request */
#define TMIDxR_TXRQ  ((uint32_t)0x00000001) /* Transmit mailbox request */

/* CAN Filter Master Register bits */
#define FMR_FINIT    ((uint32_t)0x00000001) /* Filter init mode */

/* Time out for INAK bit */
#define INAK_TIMEOUT        ((uint32_t)0x0000FFFF)
/* Time out for SLAK bit */
#define SLAK_TIMEOUT        ((uint32_t)0x0000FFFF)



/* Flags in TSR register */
#define CAN_FLAGS_TSR              ((uint32_t)0x08000000) 
/* Flags in RF1R register */
#define CAN_FLAGS_RF1R             ((uint32_t)0x04000000) 
/* Flags in RF0R register */
#define CAN_FLAGS_RF0R             ((uint32_t)0x02000000) 
/* Flags in MSR register */
#define CAN_FLAGS_MSR              ((uint32_t)0x01000000) 
/* Flags in ESR register */
#define CAN_FLAGS_ESR              ((uint32_t)0x00F00000) 

/* Mailboxes definition */
#define CAN_TXMAILBOX_0                   ((uint8_t)0x00)
#define CAN_TXMAILBOX_1                   ((uint8_t)0x01)
#define CAN_TXMAILBOX_2                   ((uint8_t)0x02) 



#define CAN_MODE_MASK              ((uint32_t) 0x00000003)


/**
 * @brief  checks whether the CAN interrupt has occurred or not.
 * @param  reg: specifies the CAN interrupt register to check.
 * @param  it_bit:  specifies the interrupt source bit to check.
 * @return TRUE: set FALSE: not set
 */
static bool CAN_IsItSet(uint32_t reg, uint32_t it_bit)
{
  return (0 != (reg & it_bit));
}



/**
  * @brief  Initializes the CAN peripheral according to the specified
  *         parameters in the CAN_Config.
  * @param  group: can group
  * @param  config: pointer to a CAN_Config structure that contains the 
  *         configuration information for the CAN peripheral.
  * @return Constant indicates initialization succeed which will be 
  *         FALSE or TRUE.
  */
bool CAN_Init(CAN_Group group, CAN_Config* config)
{
    bool ret = FALSE;
    uint32_t wait_ack = 0x00000000;
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_MODE(config->mode));
    assert_param(IS_CAN_SJW(config->sjw));
    assert_param(IS_CAN_BS1(config->bs1));
    assert_param(IS_CAN_BS2(config->bs2));
    assert_param(IS_CAN_PRESCALER(config->prescaler));

    CAN_T * const CanX = CANX[group];

    /* Exit from sleep mode */
    CanX->MCR &= (~(uint32_t)CAN_MCR_SLEEP);

    /* Request initialisation */
    CanX->MCR |= CAN_MCR_INRQ ;

    /* Wait the acknowledge */
    while (((CanX->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT))
    {
        wait_ack++;
    }

    /* Check acknowledge */
    if ((CanX->MSR & CAN_MSR_INAK) != CAN_MSR_INAK)
    {
        ret = FALSE;
    }
    else 
    {
        /* Set the time triggered communication mode */
        if (config->ttcm)
        {
            CanX->MCR |= CAN_MCR_TTCM;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_TTCM;
        }

        /* Set the automatic bus-off management */
        if (config->abom)
        {
            CanX->MCR |= CAN_MCR_ABOM;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_ABOM;
        }

        /* Set the automatic wake-up mode */
        if (config->awum)
        {
            CanX->MCR |= CAN_MCR_AWUM;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_AWUM;
        }

        /* Set the no automatic retransmission */
        if (config->nart)
        {
            CanX->MCR |= CAN_MCR_NART;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_NART;
        }

        /* Set the receive FIFO locked mode */
        if (config->rflm)
        {
            CanX->MCR |= CAN_MCR_RFLM;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_RFLM;
        }

        /* Set the transmit FIFO priority */
        if (config->txfp)
        {
            CanX->MCR |= CAN_MCR_TXFP;
        }
        else
        {
            CanX->MCR &= ~(uint32_t)CAN_MCR_TXFP;
        }

        /* Set the bit timing register */
        CanX->BTR = (uint32_t)((uint32_t)config->mode << 30) | \
                    ((uint32_t)config->sjw << 24) | \
                    ((uint32_t)config->bs1 << 16) | \
                    ((uint32_t)config->bs2 << 20) | \
                    ((uint32_t)config->prescaler - 1);

        /* Request leave initialisation */
        CanX->MCR &= ~(uint32_t)CAN_MCR_INRQ;

        /* Wait the acknowledge */
        wait_ack = 0;

        while (((CanX->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT))
        {
            wait_ack++;
        }

        /* ...and check acknowledged */
        if ((CanX->MSR & CAN_MSR_INAK) == CAN_MSR_INAK)
        {
            ret = FALSE;
        }
        else
        {
            ret = TRUE ;
        }
    }

  /* At this step, return the status of initialization */
  return ret;
}

/**
  * @brief  Initializes the CAN peripheral according to the specified
  *         parameters in the CAN_Filter.
  * @param  filter: pointer to a CAN_Filter structure that contains the 
  *                 configuration information.
  */
void CAN_FilterInit(CAN_Filter *filter)
{
    uint32_t filter_number_bit_pos = 0;
    /* Check the parameters */
    assert_param(IS_CAN_FILTER_NUMBER(filter->number));
    assert_param(IS_CAN_FILTER_MODE(filter->mode));
    assert_param(IS_CAN_FILTER_SCALE(filter->scale));
    assert_param(IS_CAN_FILTER_FIFO(filter->fifo_assignment));

    filter_number_bit_pos = ((uint32_t)1) << filter->number;

    CAN_T * const CanX = CANX[CAN1];
    /* Initialisation mode for the filter */
    CanX->FMR |= FMR_FINIT;

    /* Filter Deactivation */
    CanX->FA1R &= ~(uint32_t)filter_number_bit_pos;

    /* Filter Scale */
    if (filter->scale == CAN_FilterScale_16bit)
    {
        /* 16-bit scale for the filter */
        CanX->FS1R &= ~(uint32_t)filter_number_bit_pos;

        /* First 16-bit identifier and First 16-bit mask */
        /* Or First 16-bit identifier and Second 16-bit identifier */
        CanX->FilterRegister[filter->number].FR1 = 
            ((0x0000FFFF & (uint32_t)filter->mask_id_low) << 16) |
            (0x0000FFFF & (uint32_t)filter->id_low);

        /* Second 16-bit identifier and Second 16-bit mask */
        /* Or Third 16-bit identifier and Fourth 16-bit identifier */
        CanX->FilterRegister[filter->number].FR2 = 
            ((0x0000FFFF & (uint32_t)filter->mask_id_high) << 16) |
            (0x0000FFFF & (uint32_t)filter->id_high);
        }

        if (filter->scale == CAN_FilterScale_32bit)
        {
            /* 32-bit scale for the filter */
            CanX->FS1R |= filter_number_bit_pos;
            /* 32-bit identifier or First 32-bit identifier */
            CanX->FilterRegister[filter->number].FR1 = 
                ((0x0000FFFF & (uint32_t)filter->id_high) << 16) |
                (0x0000FFFF & (uint32_t)filter->id_low);
            /* 32-bit mask or Second 32-bit identifier */
            CanX->FilterRegister[filter->number].FR2 = 
                ((0x0000FFFF & (uint32_t)filter->mask_id_high) << 16) |
                (0x0000FFFF & (uint32_t)filter->mask_id_low);
        }

        /* Filter Mode */
        if (filter->mode == CAN_FilterMode_IdMask)
        {
            /*Id/Mask mode for the filter*/
            CanX->FM1R &= ~(uint32_t)filter_number_bit_pos;
        }
        else /* CAN_FilterInitStruct->mode == CAN_FilterMode_IdList */
        {
            /*Identifier list mode for the filter*/
            CanX->FM1R |= (uint32_t)filter_number_bit_pos;
        }

        /* Filter FIFO assignment */
        if (filter->fifo_assignment == CAN_Filter_FIFO0)
        {
            /* FIFO 0 assignation for the filter */
            CanX->FFA1R &= ~(uint32_t)filter_number_bit_pos;
        }

        if (filter->fifo_assignment == CAN_Filter_FIFO1)
        {
            /* FIFO 1 assignation for the filter */
            CanX->FFA1R |= (uint32_t)filter_number_bit_pos;
        }

        /* Filter activation */
        if (filter->activation)
        {
            CanX->FA1R |= filter_number_bit_pos;
        }

    /* Leave the initialisation mode for the filter */
    CanX->FMR &= ~FMR_FINIT;
}

/**
  * @brief  Fills each CAN_Config member with its default value.
  * @param  config: pointer to a CAN_Config structure which
  *                 will be initialized.
  */
void CAN_StructInit(CAN_Config *config)
{
    /* Reset CAN init structure parameters values */

    /* Initialize the time triggered communication mode */
    config->ttcm = FALSE;

    /* Initialize the automatic bus-off management */
    config->abom = FALSE;

    /* Initialize the automatic wake-up mode */
    config->awum = FALSE;

    /* Initialize the no automatic retransmission */
    config->nart = FALSE;

    /* Initialize the receive FIFO locked mode */
    config->rflm = FALSE;

    /* Initialize the transmit FIFO priority */
    config->txfp = FALSE;

    /* Initialize the mode member */
    config->mode = CAN_Mode_Normal;

    /* Initialize the sjw member */
    config->sjw = CAN_SJW_1tq;

    /* Initialize the bs1 member */
    config->bs1 = CAN_BS1_4tq;

    /* Initialize the bs2 member */
    config->bs2 = CAN_BS2_3tq;

    /* Initialize the prescaler member */
    config->prescaler = 1;
}

/**
  * @brief  Enables or disables the DBG Freeze for CAN.
  * @param  group: can group
  * @param  state: new state of the CAN peripheral. This parameter can 
  *                   be: ENABLE or FALSE.
  * @retval None.
  */
void CAN_DBGFreeze(CAN_Group group, bool state)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
  
    CAN_T * const CanX = CANX[group];
    if (state)
    {
        /* Enable Debug Freeze  */
        CanX->MCR |= MCR_DBF;
    }
    else
    {
        /* Disable Debug Freeze */
        CanX->MCR &= ~MCR_DBF;
    }
}


/**
  * @brief  Enables or disabes the CAN Time TriggerOperation communication mode.
  * @param  group: can group
  * @param  state : Mode new state , can be one of @ref FunctionalState.
  * @note   when enabled, Time stamp (TIME[15:0]) value is sent in the last 
  *         two data bytes of the 8-byte message: TIME[7:0] in data byte 6 
  *         and TIME[15:8] in data byte 7 
  * @note   DLC must be programmed as 8 in order Time Stamp (2 bytes) to be 
  *         sent over the CAN bus.  
  * @retval None
  */
void CAN_TTComModeCmd(CAN_Group group, bool state)
{
  /* Check the parameters */
  assert_param(group < CAN_Count);
  
  CAN_T * const CanX = CANX[group];
  if (state)
  {
    /* Enable the TTCM mode */
    CanX->MCR |= CAN_MCR_TTCM;

    /* Set TGT bits */
    CanX->TxMailBox[0].TDTR |= ((uint32_t)CAN_TDT0R_TGT);
    CanX->TxMailBox[1].TDTR |= ((uint32_t)CAN_TDT1R_TGT);
    CanX->TxMailBox[2].TDTR |= ((uint32_t)CAN_TDT2R_TGT);
  }
  else
  {
    /* Disable the TTCM mode */
    CanX->MCR &= (uint32_t)(~(uint32_t)CAN_MCR_TTCM);

    /* Reset TGT bits */
    CanX->TxMailBox[0].TDTR &= ((uint32_t)~CAN_TDT0R_TGT);
    CanX->TxMailBox[1].TDTR &= ((uint32_t)~CAN_TDT1R_TGT);
    CanX->TxMailBox[2].TDTR &= ((uint32_t)~CAN_TDT2R_TGT);
  }
}
/**
  * @brief  Initiates the transmission of a message.
  * @param  group: can group
  * @param  msg: pointer to a structure which contains CAN Id, CAN
  *                    DLC and CAN data.
  * @retval The number of the mailbox that is used for transmission
  *                    or CAN_TxStatus_NoMailBox if there is no empty mailbox.
  */
uint8_t CAN_Transmit(CAN_Group group, CAN_TxMsg *msg)
{
    uint8_t transmit_mailbox = 0;
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_IDTYPE(msg->ide));
    assert_param(IS_CAN_RTR(msg->rtr));
    assert_param(IS_CAN_DLC(msg->dlc));

    CAN_T * const CanX = CANX[group];

    /* Select one empty transmit mailbox */
    if ((CanX->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
    {
        transmit_mailbox = 0;
    }
    else if ((CanX->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
    {
        transmit_mailbox = 1;
    }
    else if ((CanX->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
    {
        transmit_mailbox = 2;
    }
    else
    {
        transmit_mailbox = CAN_TxStatus_NoMailBox;
    }

    if (transmit_mailbox != CAN_TxStatus_NoMailBox)
    {
        /* Set up the Id */
        CanX->TxMailBox[transmit_mailbox].TIR &= TMIDxR_TXRQ;
        if (msg->ide == CAN_Id_Standard)
        {
          assert_param(IS_CAN_STDID(msg->std_id));  
          CanX->TxMailBox[transmit_mailbox].TIR |= ((msg->std_id << 21) | \
                                                      msg->rtr);
        }
        else
        {
          assert_param(IS_CAN_EXTID(msg->ext_id));
          CanX->TxMailBox[transmit_mailbox].TIR |= ((msg->ext_id << 3) | \
                                                      msg->ide | \
                                                      msg->rtr);
        }

        /* Set up the DLC */
        msg->dlc &= (uint8_t)0x0000000F;
        CanX->TxMailBox[transmit_mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
        CanX->TxMailBox[transmit_mailbox].TDTR |= msg->dlc;

        /* Set up the data field */
        CanX->TxMailBox[transmit_mailbox].TDLR = (((uint32_t)msg->data[3] << 24) | 
                                                 ((uint32_t)msg->data[2] << 16) |
                                                 ((uint32_t)msg->data[1] << 8) | 
                                                 ((uint32_t)msg->data[0]));
        CanX->TxMailBox[transmit_mailbox].TDHR = (((uint32_t)msg->data[7] << 24) | 
                                                 ((uint32_t)msg->data[6] << 16) |
                                                 ((uint32_t)msg->data[5] << 8) |
                                                 ((uint32_t)msg->data[4]));
        /* Request transmission */
        CanX->TxMailBox[transmit_mailbox].TIR |= TMIDxR_TXRQ;
    }
    return transmit_mailbox;
}

/**
  * @brief  Checks the transmission of a message.
  * @param  group: can group
  * @param  TransmitMailbox: the number of the mailbox that is used for 
  *                          transmission.
  * @retval CAN_TxStatus_Ok if the CAN driver transmits the message, CAN_TxStatus_Failed 
  *         in an other case.
  */
uint8_t CAN_TransmitStatus(CAN_Group group, uint8_t TransmitMailbox)
{
  uint32_t state = 0;

    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_TRANSMITMAILBOX(TransmitMailbox));

    CAN_T * const CanX = CANX[group];
    switch (TransmitMailbox)
    {
    case (CAN_TXMAILBOX_0): 
        state = CanX->TSR &  (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);
        break;
    case (CAN_TXMAILBOX_1): 
        state = CanX->TSR &  (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);
        break;
    case (CAN_TXMAILBOX_2): 
        state = CanX->TSR &  (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);
        break;
    default:
        state = CAN_TxStatus_Failed;
        break;
    }
    switch (state)
    {
      /* transmit pending  */
    case (0x0): state = CAN_TxStatus_Pending;
        break;
    /* transmit failed  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TME0): state = CAN_TxStatus_Failed;
        break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TME1): state = CAN_TxStatus_Failed;
        break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TME2): state = CAN_TxStatus_Failed;
        break;
      /* transmit succeeded  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0):state = CAN_TxStatus_Ok;
        break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1):state = CAN_TxStatus_Ok;
        break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2):state = CAN_TxStatus_Ok;
        break;
    default: state = CAN_TxStatus_Failed;
        break;
    }
    return (uint8_t) state;
}

/**
  * @brief  Cancels a transmit request.
  * @param  group: can group
  * @param  Mailbox:  Mailbox number.
  * @retval None.
  */
void CAN_CancelTransmit(CAN_Group group, uint8_t Mailbox)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_TRANSMITMAILBOX(Mailbox));

    CAN_T * const CanX = CANX[group];
    /* abort transmission */
    switch (Mailbox)
    {
    case (CAN_TXMAILBOX_0): CanX->TSR |= CAN_TSR_ABRQ0;
        break;
    case (CAN_TXMAILBOX_1): CanX->TSR |= CAN_TSR_ABRQ1;
        break;
    case (CAN_TXMAILBOX_2): CanX->TSR |= CAN_TSR_ABRQ2;
        break;
    default:
        break;
    }
}


/**
  * @brief  Receives a message.
  * @param  group: can group
  * @param  FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
  * @param  RxMessage:  pointer to a structure receive message which contains 
  *                     CAN Id, CAN DLC, CAN datas and FMI number.
  * @retval None.
  */
void CAN_Receive(CAN_Group group, uint8_t FIFONumber, CAN_RxMsg *msg)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_FIFO(FIFONumber));

    CAN_T * const CanX = CANX[group];
    /* Get the Id */
    msg->ide = (uint8_t)0x04 & CanX->FIFOMailBox[FIFONumber].RIR;
    if (msg->ide == CAN_Id_Standard)
    {
        msg->std_id = (uint32_t)0x000007FF & (CanX->FIFOMailBox[FIFONumber].RIR >> 21);
    }
    else
    {
        msg->ext_id = (uint32_t)0x1FFFFFFF & (CanX->FIFOMailBox[FIFONumber].RIR >> 3);
    }

    msg->rtr = (uint8_t)0x02 & CanX->FIFOMailBox[FIFONumber].RIR;
    /* Get the DLC */
    msg->dlc = (uint8_t)0x0F & CanX->FIFOMailBox[FIFONumber].RDTR;
    /* Get the FMI */
    msg->fmi = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDTR >> 8);
    /* Get the data field */
    msg->data[0] = (uint8_t)0xFF & CanX->FIFOMailBox[FIFONumber].RDLR;
    msg->data[1] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDLR >> 8);
    msg->data[2] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDLR >> 16);
    msg->data[3] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDLR >> 24);
    msg->data[4] = (uint8_t)0xFF & CanX->FIFOMailBox[FIFONumber].RDHR;
    msg->data[5] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDHR >> 8);
    msg->data[6] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDHR >> 16);
    msg->data[7] = (uint8_t)0xFF & (CanX->FIFOMailBox[FIFONumber].RDHR >> 24);
    /* Release the FIFO */
    /* Release FIFO0 */
    if (FIFONumber == CAN_FIFO0)
    {
        CanX->RF0R |= CAN_RF0R_RFOM0;
    }
    /* Release FIFO1 */
    else /* FIFONumber == CAN_FIFO1 */
    {
        CanX->RF1R |= CAN_RF1R_RFOM1;
    }
}

/**
  * @brief  Releases the specified FIFO.
  * @param  group: can group
  * @param  FIFONumber: FIFO to release, CAN_FIFO0 or CAN_FIFO1.
  * @retval None.
  */
void CAN_FIFORelease(CAN_Group group, uint8_t FIFONumber)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_FIFO(FIFONumber));

    CAN_T * const CanX = CANX[group];
    /* Release FIFO0 */
    if (FIFONumber == CAN_FIFO0)
    {
        CanX->RF0R |= CAN_RF0R_RFOM0;
    }
    /* Release FIFO1 */
    else /* FIFONumber == CAN_FIFO1 */
    {
        CanX->RF1R |= CAN_RF1R_RFOM1;
    }
}

/**
  * @brief  Returns the number of pending messages.
  * @param  group: can group
  * @param  FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
  * @retval NbMessage : which is the number of pending message.
  */
uint8_t CAN_MessagePending(CAN_Group group, uint8_t FIFONumber)
{
    uint8_t message_pending=0;
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_FIFO(FIFONumber));
    
    CAN_T * const CanX = CANX[group];
    if (FIFONumber == CAN_FIFO0)
    {
        message_pending = (uint8_t)(CanX->RF0R&(uint32_t)0x03);
    }
    else if (FIFONumber == CAN_FIFO1)
    {
        message_pending = (uint8_t)(CanX->RF1R&(uint32_t)0x03);
    }
    else
    {
        message_pending = 0;
    }
    return message_pending;
}


/**
  * @brief   Select the CAN Operation mode.
  * @param CAN_OperatingMode : CAN Operating Mode. This parameter can be one 
  *                            of @ref CAN_OperatingMode_TypeDef enumeration.
  * @retval status of the requested mode which can be 
  *         - CAN_ModeStatus_Failed    CAN failed entering the specific mode 
  *         - CAN_ModeStatus_Success   CAN Succeed entering the specific mode 

  */
uint8_t CAN_OperatingModeRequest(CAN_Group group, uint8_t CAN_OperatingMode)
{
    uint8_t status = CAN_ModeStatus_Failed;

    /* Timeout for INAK or also for SLAK bits*/
    uint32_t timeout = INAK_TIMEOUT; 

    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_OPERATING_MODE(CAN_OperatingMode));

    CAN_T * const CanX = CANX[group];
    if (CAN_OperatingMode == CAN_OperatingMode_Initialization)
    {
        /* Request initialisation */
        CanX->MCR = (uint32_t)((CanX->MCR & (uint32_t)(~(uint32_t)CAN_MCR_SLEEP)) | CAN_MCR_INRQ);

        /* Wait the acknowledge */
        while (((CanX->MSR & CAN_MODE_MASK) != CAN_MSR_INAK) && (timeout != 0))
        {
            timeout--;
        }
        if ((CanX->MSR & CAN_MODE_MASK) != CAN_MSR_INAK)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else  if (CAN_OperatingMode == CAN_OperatingMode_Normal)
    {
        /* Request leave initialisation and sleep mode  and enter Normal mode */
        CanX->MCR &= (uint32_t)(~(CAN_MCR_SLEEP|CAN_MCR_INRQ));

        /* Wait the acknowledge */
        while (((CanX->MSR & CAN_MODE_MASK) != 0) && (timeout!=0))
        {
            timeout--;
        }
        if ((CanX->MSR & CAN_MODE_MASK) != 0)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else  if (CAN_OperatingMode == CAN_OperatingMode_Sleep)
    {
        /* Request Sleep mode */
        CanX->MCR = (uint32_t)((CanX->MCR & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);

        /* Wait the acknowledge */
        while (((CanX->MSR & CAN_MODE_MASK) != CAN_MSR_SLAK) && (timeout!=0))
        {
            timeout--;
        }
        if ((CanX->MSR & CAN_MODE_MASK) != CAN_MSR_SLAK)
        {
            status = CAN_ModeStatus_Failed;
        }
        else
        {
            status = CAN_ModeStatus_Success;
        }
    }
    else
    {
        status = CAN_ModeStatus_Failed;
    }

    return  (uint8_t) status;
}

/**
  * @brief  Enters the low power mode.
  * @param  group: can group
  * @retval status: CAN_Sleep_Ok if sleep entered, CAN_Sleep_Failed in an 
  *                 other case.
  */
uint8_t CAN_Sleep(CAN_Group group)
{
    uint8_t sleepstatus = CAN_Sleep_Failed;

    /* Check the parameters */
    assert_param(group < CAN_Count);

    CAN_T * const CanX = CANX[group];
    /* Request Sleep mode */
    CanX->MCR = (((CanX->MCR) & (uint32_t)(~(uint32_t)CAN_MCR_INRQ)) | CAN_MCR_SLEEP);

    /* Sleep mode status */
    if ((CanX->MSR & (CAN_MSR_SLAK|CAN_MSR_INAK)) == CAN_MSR_SLAK)
    {
        /* Sleep mode not entered */
        sleepstatus =  CAN_Sleep_Ok;
    }
    /* return sleep mode status */
    return (uint8_t)sleepstatus;
}

/**
  * @brief  Wakes the CAN up.
  * @param  group: can group
  * @retval status:  CAN_WakeUp_Ok if sleep mode left, CAN_WakeUp_Failed in an 
  *                  other case.
  */
uint8_t CAN_WakeUp(CAN_Group group)
{
    uint32_t wait_slak = SLAK_TIMEOUT;
    uint8_t wakeupstatus = CAN_WakeUp_Failed;

    /* Check the parameters */
    assert_param(group < CAN_Count);

    CAN_T * const CanX = CANX[group];
    /* Wake up request */
    CanX->MCR &= ~(uint32_t)CAN_MCR_SLEEP;

    /* Sleep mode status */
    while(((CanX->MSR & CAN_MSR_SLAK) == CAN_MSR_SLAK)&&(wait_slak!=0x00))
    {
        wait_slak--;
    }
    if((CanX->MSR & CAN_MSR_SLAK) != CAN_MSR_SLAK)
    {
        /* wake up done : Sleep mode exited */
        wakeupstatus = CAN_WakeUp_Ok;
    }
    /* return wakeup status */
    return (uint8_t)wakeupstatus;
}


/**
  * @brief  Returns the CanX's last error code (LEC).
  * @param  group: can group  
  * @retval CAN_ErrorCode: specifies the Error code : 
  *                        - CAN_ERRORCODE_NoErr            No Error  
  *                        - CAN_ERRORCODE_StuffErr         Stuff Error
  *                        - CAN_ERRORCODE_FormErr          Form Error
  *                        - CAN_ERRORCODE_ACKErr           Acknowledgment Error
  *                        - CAN_ERRORCODE_BitRecessiveErr  Bit Recessive Error
  *                        - CAN_ERRORCODE_BitDominantErr   Bit Dominant Error
  *                        - CAN_ERRORCODE_CRCErr           CRC Error
  *                        - CAN_ERRORCODE_SoftwareSetErr   Software Set Error  
  */
 
uint8_t CAN_GetLastErrorCode(CAN_Group group)
{
    uint8_t errorcode=0;

    /* Check the parameters */
    assert_param(group < CAN_Count);

    CAN_T * const CanX = CANX[group];
    /* Get the error code*/
    errorcode = (((uint8_t)CanX->ESR) & (uint8_t)CAN_ESR_LEC);

    /* Return the error code*/
    return errorcode;
}
/**
  * @brief  Returns the CanX Receive Error Counter (REC).
  * @note   In case of an error during reception, this counter is incremented 
  *         by 1 or by 8 depending on the error condition as defined by the CAN 
  *         standard. After every successful reception, the counter is 
  *         decremented by 1 or reset to 120 if its value was higher than 128. 
  *         When the counter value exceeds 127, the CAN controller enters the 
  *         error passive state.  
  * @param  group: can group 
  * @retval CAN Receive Error Counter. 
  */
uint8_t CAN_GetReceiveErrorCounter(CAN_Group group)
{
    uint8_t counter=0;

    /* Check the parameters */
    assert_param(group < CAN_Count);

    CAN_T * const CanX = CANX[group];
    /* Get the Receive Error Counter*/
    counter = (uint8_t)((CanX->ESR & CAN_ESR_REC)>> 24);

    /* Return the Receive Error Counter*/
    return counter;
}


/**
 * @brief  Returns the LSB of the 9-bit CanX Transmit Error Counter(TEC).
 * @param  group: can group 
 * @retval LSB of the 9-bit CAN Transmit Error Counter. 
 */
uint8_t CAN_GetLSBTransmitErrorCounter(CAN_Group group)
{
    uint8_t counter=0;

    /* Check the parameters */
    assert_param(group < CAN_Count);

    CAN_T * const CanX = CANX[group];
    /* Get the LSB of the 9-bit CanX Transmit Error Counter(TEC) */
    counter = (uint8_t)((CanX->ESR & CAN_ESR_TEC)>> 16);

    /* Return the LSB of the 9-bit CanX Transmit Error Counter(TEC) */
    return counter;
}


/**
  * @brief  Enables or disables the specified CanX interrupts.
* @param  group: can group.
  * @param  CAN_IT: specifies the CAN interrupt sources to be enabled or disabled.
  *                 This parameter can be: 
  *                 - CAN_IT_TME, 
  *                 - CAN_IT_FMP0, 
  *                 - CAN_IT_FF0,
  *                 - CAN_IT_FOV0, 
  *                 - CAN_IT_FMP1, 
  *                 - CAN_IT_FF1,
  *                 - CAN_IT_FOV1, 
  *                 - CAN_IT_EWG, 
  *                 - CAN_IT_EPV,
  *                 - CAN_IT_LEC, 
  *                 - CAN_IT_ERR, 
  *                 - CAN_IT_WKU or 
  *                 - CAN_IT_SLK.
  * @param  state: new state of the CAN interrupts.
  *                   This parameter can be: ENABLE or FALSE.
  * @retval None.
  */
void CAN_ITConfig(CAN_Group group, uint32_t CAN_IT, bool state)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_IT(CAN_IT));

    CAN_T * const CanX = CANX[group];
    if (state != FALSE)
    {
        /* Enable the selected CanX interrupt */
        CanX->IER |= CAN_IT;
    }
    else
    {
        /* Disable the selected CanX interrupt */
        CanX->IER &= ~CAN_IT;
    }
}

/**
  * @brief  Checks whether the specified CAN flag is set or not.
  * @param  group: can group
  * @param  CAN_FLAG: specifies the flag to check.
  *                   This parameter can be one of the following flags: 
  *                  - CAN_FLAG_EWG
  *                  - CAN_FLAG_EPV 
  *                  - CAN_FLAG_BOF
  *                  - CAN_FLAG_RQCP0
  *                  - CAN_FLAG_RQCP1
  *                  - CAN_FLAG_RQCP2
  *                  - CAN_FLAG_FMP1   
  *                  - CAN_FLAG_FF1       
  *                  - CAN_FLAG_FOV1   
  *                  - CAN_FLAG_FMP0   
  *                  - CAN_FLAG_FF0       
  *                  - CAN_FLAG_FOV0   
  *                  - CAN_FLAG_WKU 
  *                  - CAN_FLAG_SLAK  
  *                  - CAN_FLAG_LEC       
  * @retval The new state of CAN_FLAG (TRUE or FALSE).
  */
bool CAN_IsFlagSet(CAN_Group group, uint32_t CAN_FLAG)
{
    bool bitstatus = FALSE;

    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_GET_FLAG(CAN_FLAG));
  
    CAN_T * const CanX = CANX[group];
    if((CAN_FLAG & CAN_FLAGS_ESR) != (uint32_t)FALSE)
    { 
        /* Check the status of the specified CAN flag */
        if ((CanX->ESR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)FALSE)
        { 
            /* CAN_FLAG is set */
            bitstatus = TRUE;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = FALSE;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_MSR) != (uint32_t)FALSE)
    { 
        /* Check the status of the specified CAN flag */
        if ((CanX->MSR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)FALSE)
        { 
            /* CAN_FLAG is set */
            bitstatus = TRUE;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = FALSE;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_TSR) != (uint32_t)FALSE)
    { 
        /* Check the status of the specified CAN flag */
        if ((CanX->TSR & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)FALSE)
        { 
            /* CAN_FLAG is set */
            bitstatus = TRUE;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = FALSE;
        }
    }
    else if((CAN_FLAG & CAN_FLAGS_RF0R) != (uint32_t)FALSE)
    { 
        /* Check the status of the specified CAN flag */
        if ((CanX->RF0R & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)FALSE)
        { 
            /* CAN_FLAG is set */
            bitstatus = TRUE;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = FALSE;
        }
    }
    else /* If(CAN_FLAG & CAN_FLAGS_RF1R != (uint32_t)FALSE) */
    { 
        /* Check the status of the specified CAN flag */
        if ((uint32_t)(CanX->RF1R & (CAN_FLAG & 0x000FFFFF)) != (uint32_t)FALSE)
        { 
            /* CAN_FLAG is set */
            bitstatus = TRUE;
        }
        else
        { 
            /* CAN_FLAG is reset */
            bitstatus = FALSE;
        }
    }
    /* Return the CAN_FLAG status */
    return  bitstatus;
}

/**
  * @brief  Clears the CAN's pending flags.
* @param  group: can group
  * @param  CAN_FLAG: specifies the flag to clear.
  *                   This parameter can be one of the following flags: 
  *                    - CAN_FLAG_RQCP0
  *                    - CAN_FLAG_RQCP1
  *                    - CAN_FLAG_RQCP2
  *                    - CAN_FLAG_FF1       
  *                    - CAN_FLAG_FOV1   
  *                    - CAN_FLAG_FF0       
  *                    - CAN_FLAG_FOV0   
  *                    - CAN_FLAG_WKU   
  *                    - CAN_FLAG_SLAK    
  *                    - CAN_FLAG_LEC       
  * @retval None.
  */
void CAN_ClearFlag(CAN_Group group, uint32_t CAN_FLAG)
{
    uint32_t flagtmp=0;
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_CLEAR_FLAG(CAN_FLAG));

    CAN_T * const CanX = CANX[group];
    if (CAN_FLAG == CAN_FLAG_LEC) /* ESR register */
    {
        /* Clear the selected CAN flags */
        CanX->ESR = (uint32_t)FALSE;
    }
    else /* MSR or TSR or RF0R or RF1R */
    {
        flagtmp = CAN_FLAG & 0x000FFFFF;

        if ((CAN_FLAG & CAN_FLAGS_RF0R)!=(uint32_t)FALSE)
        {
            /* Receive Flags */
            CanX->RF0R = (uint32_t)(flagtmp);
        }
        else if ((CAN_FLAG & CAN_FLAGS_RF1R)!=(uint32_t)FALSE)
        {
            /* Receive Flags */
            CanX->RF1R = (uint32_t)(flagtmp);
        }
        else if ((CAN_FLAG & CAN_FLAGS_TSR)!=(uint32_t)FALSE)
        {
            /* Transmit Flags */
            CanX->TSR = (uint32_t)(flagtmp);
        }
        else /* If((CAN_FLAG & CAN_FLAGS_MSR)!=(uint32_t)FALSE) */
        {
            /* Operating mode Flags */
            CanX->MSR = (uint32_t)(flagtmp);
        }
    }
}

/**
  * @brief  Checks whether the specified CanX interrupt has occurred or not.
* @param  group: can group
  * @param  CAN_IT:  specifies the CAN interrupt source to check.
  *                  This parameter can be one of the following flags: 
  *                 -  CAN_IT_TME               
  *                 -  CAN_IT_FMP0              
  *                 -  CAN_IT_FF0               
  *                 -  CAN_IT_FOV0              
  *                 -  CAN_IT_FMP1              
  *                 -  CAN_IT_FF1               
  *                 -  CAN_IT_FOV1              
  *                 -  CAN_IT_WKU  
  *                 -  CAN_IT_SLK  
  *                 -  CAN_IT_EWG    
  *                 -  CAN_IT_EPV    
  *                 -  CAN_IT_BOF    
  *                 -  CAN_IT_LEC    
  *                 -  CAN_IT_ERR 
  * @retval The current pending state of CAN_IT (TRUE or FALSE).
  */
bool CAN_ISITPending(CAN_Group group, uint32_t CAN_IT)
{
    bool itstatus = FALSE;
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_IT(CAN_IT));
  
    CAN_T * const CanX = CANX[group];
    /* check the enable interrupt bit */
    if((CanX->IER & CAN_IT) != FALSE)
    {
        /* in case the Interrupt is enabled, .... */
        switch (CAN_IT)
        {
            case CAN_IT_TME:
                /* Check CAN_TSR_RQCPx bits */
                itstatus = CAN_IsItSet(CanX->TSR, CAN_TSR_RQCP0|CAN_TSR_RQCP1|CAN_TSR_RQCP2);  
                break;
            case CAN_IT_FMP0:
                /* Check CAN_RF0R_FMP0 bit */
                itstatus = CAN_IsItSet(CanX->RF0R, CAN_RF0R_FMP0);  
                break;
            case CAN_IT_FF0:
               /* Check CAN_RF0R_FULL0 bit */
               itstatus = CAN_IsItSet(CanX->RF0R, CAN_RF0R_FULL0);  
                break;
            case CAN_IT_FOV0:
                /* Check CAN_RF0R_FOVR0 bit */
                itstatus = CAN_IsItSet(CanX->RF0R, CAN_RF0R_FOVR0);  
                break;
            case CAN_IT_FMP1:
                /* Check CAN_RF1R_FMP1 bit */
                itstatus = CAN_IsItSet(CanX->RF1R, CAN_RF1R_FMP1);  
                break;
            case CAN_IT_FF1:
                /* Check CAN_RF1R_FULL1 bit */
                itstatus = CAN_IsItSet(CanX->RF1R, CAN_RF1R_FULL1);  
                break;
            case CAN_IT_FOV1:
                /* Check CAN_RF1R_FOVR1 bit */
                itstatus = CAN_IsItSet(CanX->RF1R, CAN_RF1R_FOVR1);  
                break;
            case CAN_IT_WKU:
                /* Check CAN_MSR_WKUI bit */
                itstatus = CAN_IsItSet(CanX->MSR, CAN_MSR_WKUI);  
                break;
            case CAN_IT_SLK:
                /* Check CAN_MSR_SLAKI bit */
                itstatus = CAN_IsItSet(CanX->MSR, CAN_MSR_SLAKI);  
                break;
            case CAN_IT_EWG:
                /* Check CAN_ESR_EWGF bit */
                itstatus = CAN_IsItSet(CanX->ESR, CAN_ESR_EWGF);  
                break;
            case CAN_IT_EPV:
                /* Check CAN_ESR_EPVF bit */
                itstatus = CAN_IsItSet(CanX->ESR, CAN_ESR_EPVF);  
                break;
            case CAN_IT_BOF:
                /* Check CAN_ESR_BOFF bit */
                itstatus = CAN_IsItSet(CanX->ESR, CAN_ESR_BOFF);  
                break;
            case CAN_IT_LEC:
                /* Check CAN_ESR_LEC bit */
                itstatus = CAN_IsItSet(CanX->ESR, CAN_ESR_LEC);  
                break;
            case CAN_IT_ERR:
                /* Check CAN_MSR_ERRI bit */ 
                itstatus = CAN_IsItSet(CanX->MSR, CAN_MSR_ERRI); 
                break;
            default :
                /* in case of error, return FALSE */
                itstatus = FALSE;
                break;
        }
    }
    else
    {
        /* in case the Interrupt is not enabled, return FALSE */
        itstatus  = FALSE;
    }

    /* Return the CAN_IT status */
    return  itstatus;
}

/**
  * @brief  Clears the CanX's interrupt pending bits.
  * @param  group: can group
  * @param  CAN_IT: specifies the interrupt pending bit to clear.
  *                  -  CAN_IT_TME                     
  *                  -  CAN_IT_FF0               
  *                  -  CAN_IT_FOV0                     
  *                  -  CAN_IT_FF1               
  *                  -  CAN_IT_FOV1              
  *                  -  CAN_IT_WKU  
  *                  -  CAN_IT_SLK  
  *                  -  CAN_IT_EWG    
  *                  -  CAN_IT_EPV    
  *                  -  CAN_IT_BOF    
  *                  -  CAN_IT_LEC    
  *                  -  CAN_IT_ERR 
  * @retval None.
  */
void CAN_ClearITPendingBit(CAN_Group group, uint32_t CAN_IT)
{
    /* Check the parameters */
    assert_param(group < CAN_Count);
    assert_param(IS_CAN_CLEAR_IT(CAN_IT));

    CAN_T * const CanX = CANX[group];
    switch (CAN_IT)
    {
        case CAN_IT_TME:
            /* Clear CAN_TSR_RQCPx (rc_w1)*/
            CanX->TSR = CAN_TSR_RQCP0|CAN_TSR_RQCP1|CAN_TSR_RQCP2;  
            break;
        case CAN_IT_FF0:
            /* Clear CAN_RF0R_FULL0 (rc_w1)*/
            CanX->RF0R = CAN_RF0R_FULL0; 
            break;
        case CAN_IT_FOV0:
            /* Clear CAN_RF0R_FOVR0 (rc_w1)*/
            CanX->RF0R = CAN_RF0R_FOVR0; 
            break;
        case CAN_IT_FF1:
            /* Clear CAN_RF1R_FULL1 (rc_w1)*/
            CanX->RF1R = CAN_RF1R_FULL1;  
            break;
        case CAN_IT_FOV1:
            /* Clear CAN_RF1R_FOVR1 (rc_w1)*/
            CanX->RF1R = CAN_RF1R_FOVR1; 
            break;
        case CAN_IT_WKU:
            /* Clear CAN_MSR_WKUI (rc_w1)*/
            CanX->MSR = CAN_MSR_WKUI;  
            break;
        case CAN_IT_SLK:
            /* Clear CAN_MSR_SLAKI (rc_w1)*/ 
            CanX->MSR = CAN_MSR_SLAKI;   
            break;
        case CAN_IT_EWG:
            /* Clear CAN_MSR_ERRI (rc_w1) */
            CanX->MSR = CAN_MSR_ERRI;
            /* Note : the corresponding Flag is cleared by hardware depending 
                of the CAN Bus status*/ 
            break;
        case CAN_IT_EPV:
            /* Clear CAN_MSR_ERRI (rc_w1) */
            CanX->MSR = CAN_MSR_ERRI; 
            /* Note : the corresponding Flag is cleared by hardware depending 
                of the CAN Bus status*/
            break;
        case CAN_IT_BOF:
            /* Clear CAN_MSR_ERRI (rc_w1) */ 
            CanX->MSR = CAN_MSR_ERRI; 
            /* Note : the corresponding Flag is cleared by hardware depending 
            of the CAN Bus status*/
            break;
        case CAN_IT_LEC:
            /*  Clear LEC bits */
            CanX->ESR = FALSE; 
            /* Clear CAN_MSR_ERRI (rc_w1) */
            CanX->MSR = CAN_MSR_ERRI; 
            break;
        case CAN_IT_ERR:
            /*Clear LEC bits */
            CanX->ESR = FALSE; 
            /* Clear CAN_MSR_ERRI (rc_w1) */
            CanX->MSR = CAN_MSR_ERRI; 
            /* Note : BOFF, EPVF and EWGF Flags are cleared by hardware depending 
            of the CAN Bus status*/
            break;
        default :
            break;
    }
}



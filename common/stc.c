#include <common_all.h>
#include <regmap.h>
#include <stc.h>
/*
 * STC_AV1 if for boot time measurement
 */
#define BOOT_TIME_STC  (STC_AV1_REG)

#define STC_COUNTER	(((u64)STC_REG->stc_63_48<<48)| \
					 ((u64)STC_REG->stc_47_32<<32)| \
					 ((u64)STC_REG->stc_31_16<<16)| \
					 ((u64)STC_REG->stc_15_0))

static void STC_hw_init(volatile struct stc_regs *regs)
{
	regs->stc_config = 0;
	regs->stc_divisor = (1 << 15) |                      /* source = EXT_REFCLK / 2 */
		((XTAL_CLK / 2 / (TIMER_KHZ * 1000)) - 1);   /* divisor = (150 -1) */
	regs->stc_31_16 = 0;
	regs->stc_15_0 = 0;
}

/*
 * STC is for drivers' delay APIs
 */
void STC_init(void)
{
	STC_hw_init(STC_REG);
}

void STC_restart(void)
{
	STC_REG->stc_31_16 = 0;
	STC_REG->stc_15_0 = 0;
}

u32 STC_Get32(void)
{
	STC_REG->stcl_2 = 0x1234;
	return (STC_REG->stcl_1 << 16) | STC_REG->stcl_0;
}

/* STC 900kHz : 1 tick = 1.11 us */
inline void STC_delay_ticks(u32 ticks)
{
	/* Detect impossible 1s delay */
	u64 counter = STC_COUNTER + ticks;

	while (STC_COUNTER < counter);
}

/* STC 900kHz : max delay = 728 ms */
void STC_delay_1ms(u32 msec)
{
	STC_delay_ticks(msec * 900);
}

/* STC 90kHz : min = 11.11us, max = 728 ms */
void STC_delay_us(u32 usec)
{
	//u32 ticks = usec / 11;
	STC_delay_ticks(usec);
}

u32 STC_get_timer(u32 base)
{
	u32 now = AV1_GetStc32();
	return (now - base)/900;
}


u32 AV1_GetStc32(void)
{
	BOOT_TIME_STC->stcl_2 = 0x1234;
	return (BOOT_TIME_STC->stcl_1 << 16) | BOOT_TIME_STC->stcl_0;
}

void AV1_STC_init(void)
{
	/* Clear STC_AV1_RESET */
	MOON0_REG->reset[1] = RF_MASK_V_CLR(1 << 6);

	STC_hw_init(BOOT_TIME_STC);
	
	STC_init();
}

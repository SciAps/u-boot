/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc functions
 */
#include <common.h>
#include <command.h>

int do_sleep (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong start = get_timer(0);
	ulong delay;

	if (argc != 2)
		return cmd_usage(cmdtp);

	delay = simple_strtoul(argv[1], NULL, 10) * CONFIG_SYS_HZ;

	while (get_timer(start) < delay) {
		if (ctrlc ())
			return (-1);

		udelay (100);
	}

	return 0;
}

U_BOOT_CMD(
	sleep ,    2,    1,     do_sleep,
	"delay execution for some time",
	"N\n"
	"    - delay execution for N seconds (N is _decimal_ !!!)"
);

#ifdef CONFIG_CMD_TIME
int do_time (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong start;
	ulong delta;
	cmd_tbl_t *cmdtp2;
	int ret;
	unsigned int secs, msecs;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	cmdtp2 = find_cmd(argv[1]);
	if (!cmdtp2) {
		printf ("Unknown command '%s' - try help\n", argv[1]);
		return 1;
	}

	start = get_timer(0);

	/* Execute command */
	ret = (cmdtp2->cmd)(cmdtp2, flag, argc-1, argv+1);

	delta = get_timer(start);

	secs = (delta * 1000) / CONFIG_SYS_HZ;
	msecs = secs % 1000;
	secs /= 1000;

	printf("'%s' took %u.%03u seconds\n", argv[1], secs, msecs);
	return ret;
}

U_BOOT_CMD(
	time ,    CONFIG_SYS_MAXARGS,    1,     do_time,
	"time execution of command",
	"command to time\n"
	"    - time execution of command in seconds"
);
#endif

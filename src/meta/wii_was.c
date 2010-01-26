#include "meta.h"
#include "../util.h"

/* WAS / ISWS

found in:	DiRT 2 (Wii)
			Formula 1 2009 (Wii)
			Sega Superstars Tennis (Wii)
*/

VGMSTREAM * init_vgmstream_wii_was(STREAMFILE *streamFile) {
    VGMSTREAM * vgmstream = NULL;
    char filename[260];
    off_t start_offset;
	int loop_flag;
	int channel_count;
	int i, j;

    /* check extension, case insensitive */
    streamFile->get_name(streamFile,filename,sizeof(filename));
    if (strcasecmp("was",filename_extension(filename)) &&
		(strcasecmp("isws",filename_extension(filename)))) goto fail;

    /* check header */
    if (read_32bitBE(0x0,streamFile) != 0x69535753) /* iSWS */
		goto fail;

    loop_flag = 0;
    channel_count = read_32bitBE(0x8,streamFile);
    
	/* build the VGMSTREAM */
    vgmstream = allocate_vgmstream(channel_count,loop_flag);
    if (!vgmstream) goto fail;

	/* fill in the vital statistics */
    start_offset = (channel_count * 0x60) + 0x20;
	vgmstream->channels = channel_count;
    vgmstream->sample_rate = read_32bitBE(0x28,streamFile);
    vgmstream->coding_type = coding_NGC_DSP;
    vgmstream->num_samples = read_32bitBE(0x20,streamFile);
    vgmstream->layout_type = layout_interleave;

	if (channel_count == 1) {
		vgmstream->layout_type = layout_none;
	} else if (channel_count > 1) {
		vgmstream->layout_type = layout_interleave;
		vgmstream->interleave_block_size = read_32bitBE(0x10,streamFile);
	}
    
	vgmstream->meta_type = meta_WII_WAS;

	{
		off_t coef_table[8] = {0x3C,0x9C,0xFC,0x15C,0x1BC,0x21C,0x27C,0x2DC};
		for (j=0;j<vgmstream->channels;j++) {
			for (i=0;i<16;i++) {
				vgmstream->ch[j].adpcm_coef[i] = read_16bitBE(coef_table[j]+i*2,streamFile);
			}
		}
	}

    /* open the file for reading */
    {
        int i;
        STREAMFILE * file;
        file = streamFile->open(streamFile,filename,STREAMFILE_DEFAULT_BUFFER_SIZE);
        if (!file) goto fail;
        for (i=0;i<channel_count;i++) {
            vgmstream->ch[i].streamfile = file;

            vgmstream->ch[i].channel_start_offset=
                vgmstream->ch[i].offset=start_offset+
                vgmstream->interleave_block_size*i;

        }
    }



    return vgmstream;

fail:
    /* clean up anything we may have opened */
    if (vgmstream) close_vgmstream(vgmstream);
    return NULL;
}

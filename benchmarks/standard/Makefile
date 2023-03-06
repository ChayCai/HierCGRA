SUBDIRS = aes bitcount cap fir fourier gsm interpol isqrt mac mac2 mults1 mults2 sha susan

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS): 
	@+make -C $@

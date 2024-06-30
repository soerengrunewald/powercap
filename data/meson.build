# SPDX-License-Identifier: LGPL-2.1-or-later

systemd_systemdsystemunitdir = get_option('systemdsystemunitdir')
install_systemdunitdir = (systemd_systemdsystemunitdir != '')

if install_systemdunitdir

  data_conf = configuration_data()
  data_conf.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))

  services = ['powercap.service.in']

  foreach service: services
    configure_file(
      input: service,
      output: '@BASENAME@',
      install_dir: systemd_systemdsystemunitdir,
      configuration: data_conf,
    )
 endforeach

endif

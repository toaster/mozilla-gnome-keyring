# encoding: UTF-8

NAME = "mozilla-gnome-keyring"
ADDON_VERSION = 1.0
FIREFOX_VERSION = 27
THUNDERBIRD_VERSION = 24

def build_dir
  File.expand_path "../build", __FILE__
end

def xpi_dir
  File.expand_path "../xpi", __FILE__
end

def lib_dir
  File.join xpi_dir, "components"
end

def lib_name(suffix = nil)
  "libgnomekeyring#{suffix && "_#{suffix}"}.so"
end

def arch
  `uname -m`.chomp.gsub /i686/, "x86"
end

def id
  "#{NAME}@pruetz.net"
end

task :build do
  # FIXME actually no build is done
end

task :install => [:build] do
  mkdir_p lib_dir
  cp File.join(build_dir, lib_name), File.join(lib_dir, lib_name(arch))
end

task :xpi => [:install, "xpi:install_rdf", "xpi:package"]

namespace :xpi do
  # TODO Generate xpi/defaults/preferences/prefs.js
  # TODO Generate xpi/chrome.manifest (and create a component UUID)
  # TODO Patch xpi/* which contains mozilla-gnome-keyring or pruetz.net
  task :install_rdf do
    File.open(File.join(xpi_dir, "install.rdf"), "w") do |f|
      f.write <<-EOF
<?xml version="1.0" encoding="UTF-8"?>
<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:em="http://www.mozilla.org/2004/em-rdf#">
  <Description about="urn:mozilla:install-manifest">
    <em:id>#{id}</em:id>
    <em:unpack>true</em:unpack>
    <em:type>2</em:type>
    <em:name>GNOME keyring password integration</em:name>
    <em:version>#{ADDON_VERSION}</em:version>
    <em:creator>Tilo Prütz</em:creator>
    <em:homepageURL>https://github.com/toaster/#{NAME}</em:homepageURL>
    <em:description>This extension enable passwords and form logins to be stored in Gnome Keyring</em:description>
    <em:aboutURL>chrome://#{NAME}/content/about.xul</em:aboutURL>
    <em:optionsURL>chrome://#{NAME}/content/options.xul</em:optionsURL>
    <em:targetApplication>
      <Description>
        <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id> <!-- Firefox -->
        <em:minVersion>13.0</em:minVersion>
        <em:maxVersion>#{FIREFOX_VERSION}</em:maxVersion>
      </Description>
    </em:targetApplication>
    <em:targetApplication>
      <Description>
        <em:id>{3550f703-e582-4d05-9a08-453d09bdfdc6}</em:id> <!-- Thunderbird -->
        <em:minVersion>13.0</em:minVersion>
        <em:maxVersion>#{THUNDERBIRD_VERSION}</em:maxVersion>
      </Description>
    </em:targetApplication>
  </Description>
*</RDF>
      EOF
    end
  end

  task :package do
    xpi = File.expand_path("../#{id}.xpi", __FILE__)
    rm_f xpi
    chdir xpi_dir do
      system "zip -r #{xpi} ."
    end
  end
end

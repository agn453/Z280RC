�$8086 prefix by agl ;-) �*�	�!��!� é
Z80 or CP/M emulator required!

BOOTSYS is a new loader for a CPM3.SYS to load and run
CP/M 3.1 in a Z80 emulator or on a real CP/M-Z80 system.

"Yet Another Z80 Emulator by AG" is a Z80 Emulator.
It is designed to provide an exact simulation of the
Z80 microprocessor on a UNIX/Linux system.

You can find yaze-ag under

	http://www.mathematik.uni-ulm.de/users/ag/yaze-ag/
     or
	ftp://ag-yaze:yaze@xylopia-upload.mathematik.uni-ulm.de

Press any key to continue ...$
BOOTSYS - CPM3.SYS, v1.23 13.01.2014 Copyright (c) 2000,2014 by A. Gerlich
$No CP/M vectors found; try to boot CP/M 3.1 using yaze-ag ...
$��* _ ����1r>��!  ݾ �ݾ�* �[ ��R��2_�{ * "`!�:� ��  !�9$ �� ��!��:_��  v�>�2 !Z6 !���:_�(r� }�0�0 fT * #~#fo>ɾ�K!]��!��!��~݆��G! T]x�(� >T͛� �>H͛����R���ݯ>N͛ !\�!��! �1r�!��V  �~�!������~�(C�g+�~� � ��������� �����!� � ����= �!��~݆�g+�V �F ���n�f����	�� ��= ��~�$��O�(�#�:_�(*`	 �v�Copyright (C) 1982, Digital Research
CP/M 3.1 is running but your BIOS does not implement XMOVE.

To load and start CP/M 3.1 from this file you must be running CP/M 2.2.

$
Loading CP/M 3.1 ...
$
CP/M 3.1 is already running and bank 1 is selected!
Now move everything to bank 0 ...
$
Everything is copied and bank 0 is selected; CPM3.SYS will be loaded. :-)
(This message is printed after a switch to bank 0)
$
No signature found.  It seems there is no CPM3.SYS attached.
Use "pip cpm3.com=bootsys.com,cpm3.sys" to attach a CPM3.SYS.

$   ����������������              �'   �          Copyright (C) 1982, Digital Research 654321                                                                     
 BIOS3    SPR  F100  0800
 BDOS3    SPR  D200  1F00
 
 52K TPA
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$�8���8̀��:����(��*���k�!������Y�*�̀��� ��
Read 
Write  error on IDE/CF device. Retry (Y/N) ? �         ��*�         ��  ���  �� ɯ!����!��2��"��͕���8�>���*�|��}��:
���>��*�� �����@ �:��� > ������ ���8!�>0������ ��� ��  �������� ����             ��  �������� ����             �  ��������  ���  �  ���  �@  �����~�"+��S-��3��~�"'��S)�! "/�!'��O��     �  �:
��'o:�g��� |e. �����             ��  �������� ����            ��O���� $(26@DHRV��������        ��������              ��  ��������  ��  �  ��3��~�"+��S-�:�*�:�*Ӣ������Ӣ�������!9y�'O�m>2��!��~��(
#� �>2���:���(�:���z��� =(�O�	�:���z�O	"��*��|eo"��:��2��͛�������O�Gy����y�͖�(�̀����y��̀���x������UART    y�������ͦ�͕�!����~�(M�W#>Ӣ>�:Ӣ�Ӣ    ��� �>�Ӣ��>�Ӣ���z���> S=                  System-mode StackSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSy��̀�>��>���    ��w�(�̀��x�̀��x����k�:��Aͳ��k��*�͞���k�*
�����	8���|�ġ��}�:���Ö́�����> �����ͳ�������w��(�j���j���{0�a8� �
Disk error on  : T= �& )c�~#fo���*�& )c�~#fo���~#fo�2�2���|�R�}�R�������~�R�;��h��[���Ɛ'�@'���ͳ�O���~#�(�Ö́�������!��.���.��n    �����. ����n    ��f    ��y2�i& )c�	~#fo��{����!��~2�!��~#fo�N���  �C���C
���C�:�2��i`z��n& �*-E   failed A:CCP.COM not loaded CCP     COM                       !���>�������͆�>�O!  ����������   ���e(�!  ���_|��g�>�2  2 !�" *��" ɯ20�go"A�!������<( ���� ���!������<(�!���k���k���k�����
,� Open Read 
%BOOT
Z280RC Non-Banked BIOS NBNK 20200630
With Cache DMACont. 
Drives CF A-D: Ram M:
  � �      1T�͈�͝�� !c��������y2��ɯ�W|   �1T�.��n>���    �e. �n!  ��������!����    �T�!V���
 �� !c��^#V#{�(��++~2�y2�+V+^��N����! ��k�͈�͝�� Ã��`��w��j�Ä��o��o��H����ð����������������K����Ö��K��K��D��T��l��u��|��p�î��o�����o��o��o��o�R�u�����                ��                          ү�     1                   O                   �       $  ���               ���       @     �{���-�>�2$��-�����:$�<�����~�2$�*��� !  "��"��++����!���ͥ����� �Æ�                                                            �6�>Ë�<w����������6 :��<� !��= ��*����´�"����*��� ��K���K����6��:��G/O�*����� *��������=�j��_����##�����:��  *������:��_�2��{�2��"���+�"�����~<�j�����5���ʖ������B�~��j�6 *���:����������!��:���i�>��������̀�������~�!��:��҈��z�}D��                                                                     ��                       #cdfg !"()       ͆��͂� �b�� ͧ��������� �b�!���B�6 ��N� �N�!����B�������! �����z�<*������"���!��"z�:���{�*��:��w :��w*���*����:����� ͯ��~�w#w#6 ##����-��a����	�*��	� ͯ�~�?G��++N�����<!��4���5͂�O 	>��p#����|��m����ͥ����Z��o��! �o�#~����*�����ͥ����� ��&���� ���������&��&�����>����͋���͹��d�� �-���j����6�͚��>�͙��F����b�����-�w���~<�q�*��>=)ү���_�|�:���0ʠ���ê�ͺ�:��<­��������H��q�:���>���~�c��!���o�#~������^#V����G#~�#~�!����##N#F#^#V#~#fo�~_*���"��}�o|�g"��>�2���{��&����&�2���{2����|�*���͆�  ��D��<��8�+}��7�*��#}�o|�g:��O�����*��s#r#w�!��~��w���~�@w����6��H��2{��&�Ó��8��:�*�����:������"���R�̀�*�����*��������8��q���̀�*��"z��:�������:������2���ͬ�͌����ͬ�͌����ͬ��(��}/_|/*��W}�o"���6��"���!_��ʍ�:��O�s�Ù�*��"�������8��c�ͬ�͌�ô�ͬ�͌�í�����ͥ������͡�͔޷�`��-��}������͔�����Pͬ� �����y�á�>O��~<�w�:��ҧ�*��
 ~�§�2���&����@����t������ͬ����͖���:z�<�ô���~�?���̀���� ������~�?�������ͥ�����*���>���͆��y�2z���p##q�:�����!�����2��!� "���R��|�:��2�����ͥ���:���N�>�2��<2��͡��|�!���w:��������>�2��*��6 �s�͙�ͤ��|��͔��>������ �N��������������	���Ä�:�����G*��` ~�!�x�_���<�_������*��y����<�:����/G���~��##~�?� �����	��@����F6 �##N6 6 ��*����L�͝� ͪ�!��~��+�6 �:���0��������'����"���!��~��6 *��>=)�X���!���f��_�������C�À�*��}�<� ~��*��~�� �{�ͧ��:��!�������f��͐�*��~���w�Ħ����ɯ2������*�� �#~�2��~�w��!  "��2��=2��*��~2���=2�������2��̀�:��*��wͳ�������6����ȇ���G�~�o�> ��_y�W#x�#��W�{��s+p+q�����r#r#r͋��S���� ��������"�͙�> 2z��4����  ���!! q#p#w�2��wW*�����_�����҆�-�!��s:z�<������! �Vq##~_��@�w͡�:z�<¸�������.������.���������������r##s��}����� �̴�� � �̭���N ! ~态O> �G~��G! ~���*���!! ~��~#~�O~��#�G~���.|���!  w:���cʇ��̈́���w�! x��?�w�! ~�ʽ����w���-����l����l�����qO������ý�������.|�W�|�*��"����������j�*����:�� ���"���R��:��W���������>������R��������:��!����� �j��x!�����w4���͚�͡�̚��@��  ���O͊�DM͠�}���>���"��"��2��:��2���*�� 	:��:���E���s�M�O 		s#r�Ͱ�:���(±�y==±���*���!��^W�*��=2��w�X��կw#���ʌ�ͺ��R��g�[�|�O�E���	��w��	��@�~�@w��> 2�����:��>��*�����͊������:��������3�:zٷ�͞����!����������>�2�� �O��b�2���@�:��G:��O��:�����=2���7�����)�y����y/W!��:����#����?�>��F6���:��_/�G�:���^�:��_�|�O��f�Ky��p�!��V���u�Wy/�2���&��:ܯ2������	�:��w++~=�w���4�-�O�ҫ�5:��<���4�������{�͊�>�2����:��!�����������3�:zٷ���͞����Ͱ����@�����R�����_��@�!  "��>�r#~���?w#>r#��=J̧�
���4�   �����w͚�� ����~O��ʙ�������:z�<�>�w##~2���_�4~�?ʈ�����>��x�:��<ʈ����ʈ��{�ͥ�������w� 	�	�ʠ�ʡ��ڠ�¡�����w��� ~�w� ��2��>��y��&�������!��"���*��>��o+"����1�͗����*���!��"���s�͙��"��� w�ɯ2z������	���̈́�ʹ����OF�q���y�����3�����Np���͆�� ͯ�:���Z�~��M�w��S�~�¹��o����������¹�#�¹�#�?����	��-��������:���@ʕ����~�#w�>�͙����{��>��� �_�++V�����*�� ����-�O��s++r �! ����x���>��w�~��6 :���6��~#�+�w#w+�� ~�����~�$��+����������*���!��>	��#>?����#��*��~ 	w����0�"����*���y�͐��}��0�͋��������͚�͙��^������2��͋�� ���l�͋������6��>� �-�͚�͙�Ç�PY*��{�z������BK��������y�ʢ�����������â�<�����y�º�!  ������>�~��G��w�#���� *��	#D�2��x���#���2��2z�G��?��S�͝�>�G���x��~��:��ʙ�2���������##~���[��*��$�a�*��"��������͋��ʄ����*���!��~�Ê�͙>�2���Í��s�͝�ͳ��Lگ2��͚�� � ͤ�͔�� �*����������'��� ����:��O ~���S�y����?��x������������-����N��G:��<��N��4���� ~*��w��:�������:��/Gy�O������A���+��B��8�y2��:��x�O���o& :��W>�O�i�E:����5����N/�����*��"��y2��͚��#��X�*���i�#����6 ����͆�DM*��6 #x�£�*��w*���*��s#r�L�*��6#6 ͝�*��"���ͤ�͔�������*��|�<�Ě�>!����>����> ��
�>�����������������w�:����͆�DM*��T]	��K���K���N��:���@��͆��*����"���N��"������ ����:���k���N �q��N#F�y�ʀ�*��}�|���w#w�͂�×�̈́���>	���!��~#��<�!��"���ͪ����*���*��#"���=�ڝ�:�������2�����6��:����F����:����������6�y�<_W`i�i�DM*��	~��N���q�*����*����*��}�<���������6���
 ^#V���ͳ�>�F�����D�>!  "��*����*���[��+�"���*���*��� �K�!z�~<�:���w�� �##>�����w�*��>�w�w�:�������*��#�*��s#r�*�����������:����!��:��G~��#����<�*��#~��!d��!��ͼ�ʷ�!��ͼ��&���!��~��6 ̀�!  "��2���*�:��o�$�*�� ~��	�6 ����~�w��	���w�*���*��{�#z���'��r+s�{�oz�g��*���*��|�g�=����v�*���*���ʄ�ʏݾ���� ��>�2��2��ͦ���#�ܨG�|�����*��ġ�:���*��w�*���*���=�ҵ�*��������>
����)ý���:��O! ͼ����*��:��O�i�}���������Á�	 ~������|�*� �*�� ����! ���~2���~��+��-�O���~���2�>�2�����:��2�����:��w:����P�4�~���:��w����r##r�p#��b��|�g}o�j�*��  ����*�� �͂�	:��ʘ�n`�	~#fo��b�2��O ͊�"��}��:��O*�����"��2��"��:��O:��G2��!��w�*�� ~��zW�w+���z����#~�*��! �*��|�y����#�������*��Ͱ������*��DM"�����*��s#r��*��s#r#p�y�ox�g�[�DM*����0�MD" ��!�*��MD�$�:��O�i�!��N:���i�G>�O:��ʀ۷�wۍ�<�*�������p��:���0�!��:��®����:���Íٯ2���*���i�"�� �!��s#r#p�*��N#F�*��^#V#F*��:��O}�|�y�����*��Ͱ���+����*��͹��}���|��^#V#"��##"��##"��###"��#"��##�"��!�����*���!�����*��|!��6���J�6 7���*��w#w*��w#w#w�>2���'��qگ2���*��O͂�y�ڍ���Í�>�2��a.�"z�:��2��:��<���:����^���^��d�{�_z�W��{�_z�W���)�������4�#4�#4����������������������#�����w#���J�o������͠��#����    CP/M Error On  : $Disk I/O$Invalid Drive$Read/Only File$Read/Only Disk$File Exists$? in Filename$  Í���͛շ��S�����6�����9�����MD���:�����!���:��������	�����2ٯ����Ͱ�2z��>���z�<*������"���!��z�<~���s��^#V#N#F�x����N:����������:ٷ�J�!��2����!�5���J��x��4�������J��#w��O͏���:����K�~�x�T������H��p!  "���I������y<ʃ�<�z�<ʈ���͛��&��:��<�e�x��H�~+�8��¨��x2����֯2��J��¸��ů�����H������:�!���-�5���þ����������-���#د2���������x���#N��z�W��:ٺ���*��}��S�~#���!  "��"����S�z��-�*��"��##"��>2��:��2�*��N#�G2ٹ�H���������W��
�W���|�:��<ʈ�:�W:���H�2�x��y������y��5���6 �y͌�Ҡ��^�I���@O:��=ʯ�:����I�y�	�I� �I�:���µ����� ����#�I����:��!پ� �I�����I�
�I�!��
���O͠������
�>��x������w>ɯw��%����:����7�!��>��wɯ�:��=��x�����:ٷ�r�:���G�:��=Ľ����9��x��r��:�����y!����4� �5~����b���b�����
��	���� �:ٷ������G:���x�ͨ��:ٷ���ͨ��:����������	�����!پ���6 �	�����:�����ԯ��<�����!�G:o�  ����������  :��O*����7�*��� �/�*��"��*�"���[��<�:��g��:���!����,��*���}D�!�~6 ���	�!����S�͌��r��O͠���ͨ��������bxԷ���a��{ҏ��_��p#��� ʙ��	ʙ������:���A2'���������!������%�"����  !  9"�1�!>������%�:���2���շ�5�:���!������=���g�?������͙��s���!  ��������!����� 	�?��+�>��� 	w���s��� �����X��*�]�w#��6?#�]��	 .,:;[]=<>| �!eԾʃ�45#�l����!���*��^#V�#^#V���w# ͒� ͒� ͒� ͒�͙��:������s����A�����<w#�s��� �.����;�!��?��������	 	 �;�!�נ���+�5�����(�.�7�@������ݫ�����������U���������&������%�*�g�y�������&�����&�%�2���G�P���������+�Q�B�4�    `�`�p�		^#V�ͦ��:���ü�!"( (C) 98 CALDERA101198����������������������������������������������������������������^ՠ�d���j؎ؚؔ�654321�"���y2����7�!  "��:��2��:��=�7�!��~��7ҹ���#�*�{2��!  "z�"��9"��1��!d��y�3�\�K!���n���҂��b�&���&�!R�_ ^#V*���� �!
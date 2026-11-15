#!/usr/bin/env python3
"""Check JSON and quiet-mode contracts using separate stdout/stderr streams.

Run with: python3 tests/cli/json-report.py [path/to/dvdisaster]
Uses only the Python standard library; all fixtures are disposable.
"""
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

BINARY = str(Path(sys.argv.pop(1) if len(sys.argv) > 1 else './dvdisaster').resolve())


class Reports(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='dvdisaster-json-')
        cls.root = Path(cls.temp.name)
        cls.iso = cls.root / 'master.iso'
        cls.ecc = cls.root / 'master.ecc'
        cls.missing = cls.root / 'absent'
        cls.run_cli('--debug', '--random-image', '1000', '-i', cls.iso)
        cls.run_cli('-c', '-i', cls.iso, '-e', cls.ecc)
        cls.damaged = cls.root / 'damaged.iso'
        shutil.copyfile(cls.iso, cls.damaged)
        cls.run_cli('--debug', '--erase', '500', '-i', cls.damaged)

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    @staticmethod
    def run_cli(*args, **kwargs):
        return subprocess.run([BINARY, '--regtest', '--no-progress', *map(str, args)],
                              stderr=subprocess.PIPE,
                              stdout=kwargs.pop('stdout', subprocess.PIPE),
                              timeout=60, **kwargs)

    def verify(self, *args, image=None, ecc=None, **kwargs):
        return self.run_cli('-t', '-i', image or self.iso, '-e', ecc or self.ecc,
                            *args, **kwargs)

    def test_quiet_json_and_unicode(self):
        path = self.root / 'quote"-backslash\\-tab\t-newline\n-café-光.iso'
        # Windows forbids some of these filename characters.
        if os.name == 'nt':
            path = self.root / 'café-光.iso'
        shutil.copyfile(self.iso, path)
        for flags in [('-q',), ('-q', '-q'), ('-qq',), ('--quiet', '--quiet')]:
            with self.subTest(flags=flags):
                p = self.verify(*flags, '--json', '-', image=path)
                self.assertEqual(p.returncode, 0)
                self.assertEqual(p.stderr, b'')
                report = json.loads(p.stdout)
                self.assertEqual(report['image']['path'], str(path))
                self.assertEqual(report['verdict'], 'ok')
                self.assertEqual(report['verify']['missing_sectors'], 0)

    def test_utc_timestamp(self):
        # The report must remain UTC even when the process uses another zone.
        env = dict(os.environ, TZ='UTC+11')
        before = datetime.now(timezone.utc).replace(microsecond=0)
        p = self.verify('-q', '--json', '-', env=env)
        after = datetime.now(timezone.utc)
        self.assertEqual(p.returncode, 0)
        self.assertEqual(p.stderr, b'')
        timestamp = json.loads(p.stdout)['time']
        self.assertRegex(timestamp, r'^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$')
        parsed = datetime.strptime(timestamp, '%Y-%m-%dT%H:%M:%SZ').replace(tzinfo=timezone.utc)
        self.assertLessEqual(before, parsed)
        self.assertLessEqual(parsed, after)

    def test_exclusive_output(self):
        report = self.root / 'report.json'
        p = self.verify('-q', '--json', report)
        self.assertEqual(p.returncode, 0)
        self.assertEqual(p.stderr, b'')
        self.assertEqual(json.loads(report.read_bytes())['exit_code'], 0)
        for path in [report, self.iso, self.ecc]:
            with self.subTest(path=path):
                before = path.read_bytes()
                p = self.verify('-q', '--json', path)
                self.assertEqual(p.returncode, 0)
                self.assertIn(b'Could not write JSON report', p.stderr)
                self.assertEqual(path.read_bytes(), before)

    @unittest.skipIf(os.name == 'nt', 'Unix filesystem links')
    def test_links_are_not_followed(self):
        for name, target in [('alias', self.iso), ('dangling', self.missing)]:
            path = self.root / name
            path.symlink_to(target)
            p = self.verify('-q', '--json', path)
            self.assertEqual(p.returncode, 0)
            self.assertIn(b'Could not write JSON report', p.stderr)
            self.assertTrue(path.is_symlink())
        self.assertFalse(self.missing.exists())
        hardlink = self.root / 'hardlink'
        os.link(self.iso, hardlink)
        before = self.iso.read_bytes()
        p = self.verify('-q', '--json', hardlink)
        self.assertIn(b'Could not write JSON report', p.stderr)
        self.assertEqual(self.iso.read_bytes(), before)

    def test_output_errors_preserve_operation_status(self):
        for flags, expected in [(('-q',), 1), (('-q', '--detailed-exit-status'), 2),
                                (('-qq', '--detailed-exit-status'), 2)]:
            p = self.verify(*flags, '--json', self.missing / 'report.json',
                            image=self.damaged)
            self.assertEqual(p.returncode, expected)
            if '-qq' in flags:
                self.assertEqual(p.stderr, b'')
            else:
                self.assertIn(b'Could not write JSON report', p.stderr)

    @unittest.skipUnless(Path('/dev/full').exists(), 'requires /dev/full')
    def test_stdout_flush_failure(self):
        for quiet in ['-q', '-qq']:
            with open('/dev/full', 'wb') as full:
                p = self.verify(quiet, '--json', '-', stdout=full)
            self.assertEqual(p.returncode, 0)
            if quiet == '-q':
                self.assertIn(b'Could not write JSON report', p.stderr)
            else:
                self.assertEqual(p.stderr, b'')

    @unittest.skipIf(os.name == 'nt', 'requires Unix resource limits')
    def test_file_close_failure(self):
        import resource
        import signal

        def limit_file():
            signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
            resource.setrlimit(resource.RLIMIT_FSIZE, (16, 16))

        path = self.root / 'partial.json'
        p = self.verify('-q', '--json', path, preexec_fn=limit_file)
        self.assertEqual(p.returncode, 0)
        self.assertIn(b'Could not write JSON report', p.stderr)
        self.assertLessEqual(path.stat().st_size, 16)

    @unittest.skipIf(os.name == 'nt', 'requires Unix pipe semantics')
    def test_closed_stdout_pipe(self):
        read_fd, write_fd = os.pipe()
        os.close(read_fd)
        with os.fdopen(write_fd, 'wb') as pipe:
            p = self.verify('-q', '--json', '-', stdout=pipe)
        self.assertEqual(p.returncode, 0)
        self.assertIn(b'Could not write JSON report', p.stderr)

    @unittest.skipIf(os.name == 'nt', 'Unix filenames can contain arbitrary bytes')
    def test_invalid_utf8(self):
        raw_path = os.fsencode(self.root) + b'/invalid-\xe9-\xe2\x82.iso'
        os.symlink(os.fsencode(self.iso), raw_path)
        p = self.verify('-q', '--json', '-', image=os.fsdecode(raw_path))
        self.assertEqual(p.returncode, 0)
        report = json.loads(p.stdout)
        self.assertEqual(report['image']['path'],
                         str(self.root) + '/invalid-\ufffd-\ufffd\ufffd.iso')

    def test_missing_inputs_diagnostics(self):
        for action in ['-t', '-f']:
            for quiet in ['-q', '-qq']:
                p = self.run_cli(action, quiet, '-i', self.missing,
                                 '-e', self.missing, '--json', '-')
                self.assertEqual(p.returncode, 1)
                self.assertEqual(json.loads(p.stdout)['verdict'], 'error')
                if quiet == '-q':
                    self.assertIn(b'not present', p.stderr)
                else:
                    self.assertEqual(p.stderr, b'')

    def test_invalid_ecc_diagnostics(self):
        invalid = self.root / 'invalid.ecc'
        invalid.write_bytes(b'not an ECC file')
        p = self.verify('-q', '--json', '-', ecc=invalid)
        self.assertEqual(p.returncode, 1)
        self.assertIn(b'invalid', p.stderr)
        self.assertEqual(json.loads(p.stdout)['exit_code'], 1)

    @unittest.skipIf(os.name == 'nt', 'requires ENOTDIR for a path through a file')
    def test_image_open_error_with_valid_ecc(self):
        path = self.iso / 'child.iso'
        for quiet in ['-q', '-qq']:
            p = self.verify(quiet, '--json', '-', image=path)
            self.assertEqual(p.returncode, 1)
            self.assertEqual(json.loads(p.stdout)['exit_code'], 1)
            if quiet == '-q':
                self.assertIn(b'Could not open', p.stderr)
            else:
                self.assertEqual(p.stderr, b'')

    def test_detailed_verdict_and_defect_diagnostics(self):
        for quiet in ['-q', '-qq']:
            p = self.verify(quiet, '--detailed-exit-status', '--json', '-',
                            image=self.damaged, ecc=self.missing)
            self.assertEqual(p.returncode, 3)
            report = json.loads(p.stdout)
            self.assertEqual(report['verdict'], 'incomplete')
            self.assertEqual(report['verify']['missing_sectors'], 1)
            if quiet == '-q':
                self.assertIn(b'sectors missing', p.stderr)
            else:
                self.assertEqual(p.stderr, b'')

    def test_chained_actions(self):
        image = self.root / 'repair.iso'
        shutil.copyfile(self.damaged, image)
        p = self.run_cli('-f', '-t', '-q', '--detailed-exit-status', '-i', image,
                         '-e', self.ecc, '--json', '-')
        self.assertEqual(p.returncode, 2)
        report = json.loads(p.stdout)
        self.assertEqual(report['actions'], ['fix', 'verify'])
        self.assertEqual(report['verdict'], 'defects')
        self.assertEqual(report['fix']['repaired'], 1)
        self.assertEqual(report['verify']['missing_sectors'], 0)
        self.assertEqual(image.read_bytes(), self.iso.read_bytes())

    def test_scan_diagnostics(self):
        env = dict(os.environ, DVDISASTER_SCSI_SIMULATED_NODELAY='1')
        for quiet in ['-q', '-qq']:
            p = self.run_cli('-s', quiet, '--debug',
                             '--sim-cd=' + str(self.damaged), '--spinup-delay=0',
                             '-e', self.ecc, '--json', '-', env=env)
            self.assertEqual(p.returncode, 1)
            self.assertGreater(json.loads(p.stdout)['scan']['unreadable'], 0)
            if quiet == '-q':
                self.assertIn(b'unreadable sectors', p.stderr)
                self.assertIn(b'Medium Error', p.stderr)
            else:
                self.assertEqual(p.stderr, b'')

    def test_warning_suppression(self):
        # Repairing a truncated image emits a ModalWarning.
        for quiet in ['-q', '-qq']:
            image = self.root / ('truncated' + quiet + '.iso')
            shutil.copyfile(self.iso, image)
            with image.open('r+b') as out:
                out.truncate(image.stat().st_size - 2048)
            p = self.run_cli('-f', quiet, '--truncate', '-i', image,
                             '-e', self.ecc, '--json', '-')
            self.assertEqual(p.returncode, 0)
            self.assertEqual(json.loads(p.stdout)['exit_code'], 0)
            if quiet == '-q':
                self.assertIn(b'Warning', p.stderr)
            else:
                self.assertEqual(p.stderr, b'')
            self.assertEqual(image.read_bytes(), self.iso.read_bytes())


if __name__ == '__main__':
    unittest.main()

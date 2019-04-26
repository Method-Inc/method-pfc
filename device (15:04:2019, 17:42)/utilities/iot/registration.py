# Import standard python modules
import os, subprocess, shutil

# Import device utilities
from device.utilities.logger import Logger
from device.utilities import network

# Initialize file paths
REGISTRATION_DATA_DIR = "data/registration/"
DEVICE_ID_PATH = "data/registration/device_id.bash"
ROOTS_PATH = "data/registration/roots.pem"
RSA_CERT_PATH = "data/registration/rsa_cert.pem"
RSA_PRIVATE_PATH = "data/registration/rsa_private.pem"
VERIFICATION_CODE_PATH = "data/registration/verification_code.txt"
REGISTER_SCRIPT_PATH = "scripts/iot/one_time_key_creation_and_iot_device_registration.sh"

# Initialize logger
logger = Logger("IotRegistrationUtility", "iot")


def is_registered() -> bool:
    """Checks if device is registered by checking local files."""
    logger.debug("Checking if device is registered")
    if (
        os.path.exists(DEVICE_ID_PATH)
        and os.path.exists(ROOTS_PATH)
        and os.path.exists(RSA_CERT_PATH)
        and os.path.exists(RSA_PRIVATE_PATH)
    ):
        return True
    else:
        return False


def device_id() -> str:
    """Gets device id string from local file. TODO: Handle exeptions."""
    logger.debug("Getting device id")
    try:
        with open(DEVICE_ID_PATH) as f:
            contents = f.read()
            index = contents.find("=")
            device_id = contents[index + 1:].strip()
            return device_id
    except FileNotFoundError:
        return "UNKNOWN"
    except Exception as e:
        message = "Unable to get device id, unhandled exception: {}".format(type(e))
        logger.exception(message)
        return "UNKNOWN"


def verification_code() -> str:
    """Gets verification code from local file. TODO: Handle exceptions."""
    logger.debug("Getting verification code")
    try:
        with open(VERIFICATION_CODE_PATH) as f:
            verification_code = f.read().strip()
            return verification_code
    except FileNotFoundError:
        return "INVALID"
    except Exception as e:
        message = "Unable to get verification code, unhandled exception: {}".format(
            type(e)
        )
        logger.exception(message)
        return "INVALID"


def register() -> None:
    """Registers device with Google IoT. Stores registration data in local files."""
    logger.debug("Registering device with iot")

    # Check if already registed
    if is_registered():
        logger.error("Unable to register, already registered")
        return

    # Check network is connected
    if not network.is_connected():
        logger.warning("Unable to register, network is not connected")
        return

    # Build commands
    make_directory_command = ["mkdir", "-p", REGISTRATION_DATA_DIR]
    register_command = [REGISTER_SCRIPT_PATH, REGISTRATION_DATA_DIR]

    # Execute commands
    try:
        subprocess.run(make_directory_command)
        subprocess.run(register_command)
    except Exception as e:
        message = "Unable to register, unhandled exception: {}".format(type(e))
        logger.exception(message)


def delete() -> None:
    """Deletes local registration data."""
    logger.debug("Deleting registration data")

    if os.path.exists(REGISTRATION_DATA_DIR):
        shutil.rmtree(REGISTRATION_DATA_DIR)
